#include "raytracer/attenuator.h"
#include "raytracer/construct_impulse.h"
#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflector.h"

#include "common/azimuth_elevation.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/dsp_vector_ops.h"
#include "common/map_to_vector.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/string_builder.h"
#include "common/write_audio_file.h"

#include "gtest/gtest.h"

#include <set>

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef SCRATCH_PATH
#define SCRATCH_PATH ""
#endif

constexpr auto speed_of_sound{340.0};
constexpr auto acoustic_impedance{400.0};
constexpr auto bench_reflections{128};
constexpr auto bench_rays{1 << 15};

const glm::vec3 source{1, 2, 1};
const auto the_rays = get_random_directions(bench_rays);

TEST(raytrace, new) {
    const compute_context cc;

    const scene_data scene(OBJ_PATH);
    const voxelised_scene_data voxelised(
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1}));

    std::atomic_bool keep_going{true};
    auto results = raytracer::run(cc,
                                  voxelised,
                                  speed_of_sound,
                                  source,
                                  glm::vec3(0, 1.75, 0),
                                  the_rays,
                                  bench_reflections,
                                  10,
                                  keep_going,
                                  [](auto) {});

    ASSERT_TRUE(results);
}

constexpr int width_for_shell(int shell) { return shell * 2 + 1; }

constexpr int num_images(int shell) {
    auto w = width_for_shell(shell);
    return w * w * w;
}

template <int SHELL>
std::array<glm::vec3, num_images(SHELL)> images_for_shell(
        const geo::box& box, const glm::vec3& source) {
    std::array<glm::vec3, num_images(SHELL)> ret;

    auto image = source;

    constexpr auto L = width_for_shell(SHELL);
    for (int i = 0; i != L; ++i) {
        auto x = i - SHELL;
        auto reflected_x =
                x % 2 ? mirror_inside(box, image, geo::direction::x) : image;
        for (int j = 0; j != L; ++j) {
            auto y = j - SHELL;
            auto reflected_y =
                    y % 2 ? mirror_inside(box, reflected_x, geo::direction::y)
                          : reflected_x;
            for (int k = 0; k != L; ++k) {
                auto z = k - SHELL;
                auto reflected_z =
                        z % 2 ? mirror_inside(
                                        box, reflected_y, geo::direction::z)
                              : reflected_y;

                ret[i + j * L + k * L * L] =
                        reflected_z + glm::vec3(x, y, z) * dimensions(box);
            }
        }
    }

    return ret;
}

TEST(raytrace, same_location) {
    geo::box box(glm::vec3(0, 0, 0), glm::vec3(4, 3, 6));
    auto receiver = source;
    constexpr auto s = 0.9;
    constexpr auto d = 0.1;
    constexpr surface surface{volume_type{{s, s, s, s, s, s, s, s}},
                              volume_type{{d, d, d, d, d, d, d, d}}};

    auto scene = geo::get_scene_data(box);
    scene.set_surfaces(surface);
    const voxelised_scene_data voxelised(
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1}));

    compute_context cc;

    auto callback_count{0};

    std::atomic_bool keep_going{true};
    const auto results =
            raytracer::run(cc,
                           voxelised,
                           speed_of_sound,
                           source,
                           receiver,
                           the_rays,
                           bench_reflections,
                           10,
                           keep_going,
                           [&](auto i) { ASSERT_EQ(i, callback_count++); });

    ASSERT_TRUE(results);

    const auto diffuse = results->get_diffuse();

    for (auto i = 0u; i != bench_rays; ++i) {
        const auto intersection =
                intersects(voxelised, geo::ray{source, the_rays[i]});
        if (intersection) {
            const auto cpu_position =
                    source + (the_rays[i] * intersection->inter.t);
            const auto gpu_position = to_vec3(diffuse[i][0].position);
            if (!nearby(cpu_position, gpu_position, 0.0001)) {
                ;
            }
        }
    }
}

TEST(raytrace, image_source) {
    //  proper method
    const geo::box box(glm::vec3(0, 0, 0), glm::vec3(4, 3, 6));
    constexpr glm::vec3 receiver(2, 1, 5);
    constexpr auto s = 0.9;
    constexpr auto d = 0.1;
    constexpr surface surface{volume_type{{s, s, s, s, s, s, s, s}},
                              volume_type{{d, d, d, d, d, d, d, d}}};

    constexpr auto shells = 3;
    auto images = images_for_shell<shells>(box, source);
    std::array<float, images.size()> distances;
    proc::transform(images, distances.begin(), [&receiver](auto i) {
        return glm::distance(receiver, i);
    });
    std::array<float, images.size()> times;
    proc::transform(distances, times.begin(), [](auto i) {
        return i / speed_of_sound;
    });
    std::array<volume_type, images.size()> volumes;
    proc::transform(distances, volumes.begin(), [](auto i) {
        return raytracer::attenuation_for_distance(i);
    });

    aligned::vector<attenuated_impulse> proper_image_source_impulses;

    constexpr auto L = width_for_shell(shells);
    for (int i = 0; i != L; ++i) {
        for (int j = 0; j != L; ++j) {
            for (int k = 0; k != L; ++k) {
                auto shell_dim = glm::vec3(std::abs(i - shells),
                                           std::abs(j - shells),
                                           std::abs(k - shells));
                auto reflections = shell_dim.x + shell_dim.y + shell_dim.z;
                if (reflections <= shells) {
                    auto index = i + j * L + k * L * L;
                    auto volume = volumes[index];
                    auto base_vol = pow(-s, reflections);
                    volume *= base_vol;

                    proper_image_source_impulses.push_back(
                            attenuated_impulse{volume, times[index]});
                }
            }
        }
    }

    auto sort_by_time = [](auto& i) {
        proc::sort(i, [](auto a, auto b) { return a.time < b.time; });
    };

    sort_by_time(proper_image_source_impulses);

    //  raytracing method
    const auto cc{compute_context{}};

    auto scene = geo::get_scene_data(box);
    scene.set_surfaces(surface);
    const voxelised_scene_data voxelised(
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1}));

    std::atomic_bool keep_going{true};
    auto results{raytracer::run(cc,
                                voxelised,
                                speed_of_sound,
                                source,
                                receiver,
                                the_rays,
                                100,
                                10,
                                keep_going,
                                [](auto) {})};

    ASSERT_TRUE(results);

    auto output = results->get_impulses(true, true, false);

    ASSERT_TRUE(output.size() > 1);

    sort_by_time(output);

    for (auto i : proper_image_source_impulses) {
        auto closest = std::accumulate(
                output.begin() + 1,
                output.end(),
                output.front(),
                [i](auto a, auto b) {
                    return std::abs(a.time - i.time) < std::abs(b.time - i.time)
                                   ? a
                                   : b;
                });
        ASSERT_NEAR(i.time, closest.time, 0.001);
    }

    auto postprocess = [](const auto& i, const std::string& name) {
        const auto bit_depth = 16;
        const auto sample_rate = 44100.0;
        {
            auto mixed_down = mixdown(raytracer::flatten_impulses(
                    i, sample_rate, acoustic_impedance));
            normalize(mixed_down);
            snd::write(
                    build_string(SCRATCH_PATH, "/", name, "_no_processing.wav"),
                    {mixed_down},
                    sample_rate,
                    bit_depth);
        }
        {
            auto processed = raytracer::flatten_filter_and_mixdown(
                    i, sample_rate, acoustic_impedance);
            normalize(processed);
            snd::write(build_string(SCRATCH_PATH, "/", name, "_processed.wav"),
                       {processed},
                       sample_rate,
                       bit_depth);
        }
    };

    postprocess(output, "raytraced");
    postprocess(proper_image_source_impulses, "image_src");
    postprocess(results->get_impulses(false, false, true), "diffuse");
    postprocess(results->get_impulses(), "full");
}
