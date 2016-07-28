#include "raytracer/attenuator.h"
#include "raytracer/construct_impulse.h"
#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"

#include "common/boundaries.h"
#include "common/cl_common.h"
#include "common/map.h"
#include "common/string_builder.h"
#include "common/write_audio_file.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

#include <set>

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef SCRATCH_PATH
#define SCRATCH_PATH ""
#endif

static constexpr auto bench_reflections = 128;
static constexpr auto bench_rays        = 1 << 15;

TEST(raytrace, new) {
    compute_context cc;
    raytracer_program raytracer_program(cc.get_context(), cc.get_device());

    SceneData scene_data(OBJ_PATH);

    std::atomic_bool keep_going{true};
    raytracer::raytracer raytrace(cc.get_context(), cc.get_device());
    auto results = raytrace.run(scene_data,
                                glm::vec3(0, 1.75, 3),
                                glm::vec3(0, 1.75, 0),
                                bench_rays,
                                bench_reflections,
                                10,
                                keep_going,
                                [] {});

    ASSERT_TRUE(results);
}

constexpr int width_for_shell(int shell) { return shell * 2 + 1; }

constexpr int num_images(int shell) {
    auto w = width_for_shell(shell);
    return w * w * w;
}

template <int SHELL>
std::array<glm::vec3, num_images(SHELL)> images_for_shell(
        const box& box, const glm::vec3& source) {
    std::array<glm::vec3, num_images(SHELL)> ret;

    auto image = source;

    constexpr auto L = width_for_shell(SHELL);
    for (int i = 0; i != L; ++i) {
        auto x = i - SHELL;
        auto reflected_x =
                x % 2 ? mirror_inside(box, image, direction::x) : image;
        for (int j = 0; j != L; ++j) {
            auto y = j - SHELL;
            auto reflected_y =
                    y % 2 ? mirror_inside(box, reflected_x, direction::y)
                          : reflected_x;
            for (int k = 0; k != L; ++k) {
                auto z = k - SHELL;
                auto reflected_z =
                        z % 2 ? mirror_inside(box, reflected_y, direction::z)
                              : reflected_y;

                ret[i + j * L + k * L * L] =
                        reflected_z + glm::vec3(x, y, z) * dimensions(box);
            }
        }
    }

    return ret;
}

TEST(raytrace, image_source) {
    //  proper method
    box box(glm::vec3(0, 0, 0), glm::vec3(4, 3, 6));
    constexpr glm::vec3 source(1, 2, 1);
    constexpr glm::vec3 receiver(2, 1, 5);
    constexpr auto s = 0.9;
    constexpr auto d = 0.1;
    constexpr Surface surface{VolumeType{{s, s, s, s, s, s, s, s}},
                              VolumeType{{d, d, d, d, d, d, d, d}}};

    constexpr auto shells = 3;
    auto images           = images_for_shell<shells>(box, source);
    std::array<float, images.size()> distances;
    proc::transform(images, distances.begin(), [&receiver](auto i) {
        return glm::distance(receiver, i);
    });
    std::array<float, images.size()> times;
    proc::transform(distances, times.begin(), [](auto i) {
        return i / SPEED_OF_SOUND;
    });
    std::array<VolumeType, images.size()> volumes;
    proc::transform(distances, volumes.begin(), [](auto i) {
        return attenuation_for_distance(i);
    });

    aligned::vector<AttenuatedImpulse> proper_image_source_impulses;

    constexpr auto L = width_for_shell(shells);
    for (int i = 0; i != L; ++i) {
        for (int j = 0; j != L; ++j) {
            for (int k = 0; k != L; ++k) {
                auto shell_dim = glm::vec3(std::abs(i - shells),
                                           std::abs(j - shells),
                                           std::abs(k - shells));
                auto reflections = shell_dim.x + shell_dim.y + shell_dim.z;
                if (reflections <= shells) {
                    auto index    = i + j * L + k * L * L;
                    auto volume   = volumes[index];
                    auto base_vol = pow(-s, reflections);
                    for (auto band = 0; band != 8; ++band)
                        volume.s[band] *= base_vol;

                    proper_image_source_impulses.push_back(
                            AttenuatedImpulse{volume, times[index]});
                }
            }
        }
    }

    auto sort_by_time = [](auto& i) {
        proc::sort(i, [](auto a, auto b) { return a.time < b.time; });
    };

    sort_by_time(proper_image_source_impulses);

    //  raytracing method
    compute_context cc;

    raytracer::raytracer raytracer(cc.get_context(), cc.get_device());

    auto scene_data = get_scene_data(box);
    scene_data.set_surfaces(surface);

    std::atomic_bool keep_going{true};
    auto results = raytracer.run(
            scene_data, source, receiver, 100000, 100, 10, keep_going, [] {});

    ASSERT_TRUE(results);

    const auto convert = [](const auto& input) {
        return map_to_vector(input, [](const auto& i) {
            return AttenuatedImpulse{i.volume, i.time};
        });
    };

    auto output = convert(results->get_impulses(true, true, false));
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
        const auto bit_depth   = 16;
        const auto sample_rate = 44100.0;
        {
            auto mixed_down =
                    mixdown(raytracer::flatten_impulses(i, sample_rate));
            normalize(mixed_down);
            snd::write(
                    build_string(SCRATCH_PATH, "/", name, "_no_processing.wav"),
                    {mixed_down},
                    sample_rate,
                    bit_depth);
        }
        {
            auto processed =
                    raytracer::flatten_filter_and_mixdown(i, sample_rate);
            normalize(processed);
            snd::write(build_string(SCRATCH_PATH, "/", name, "_processed.wav"),
                       {processed},
                       sample_rate,
                       bit_depth);
        }
    };

    postprocess(output, "raytraced");
    postprocess(proper_image_source_impulses, "image_src");
    postprocess(convert(results->get_impulses()), "diffuse");
}
