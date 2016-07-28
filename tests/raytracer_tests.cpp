#include "raytracer/attenuator.h"
#include "raytracer/raytracer.h"
#include "raytracer/construct_impulse.h"

#include "common/boundaries.h"
#include "common/cl_common.h"
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

/*
#define USE_EPSILON

#ifdef USE_EPSILON
const auto EPSILON = 0.00001;
#endif

bool operator==(const cl_float3& a, const cl_float3& b) {
    for (auto i = 0u; i != 3; ++i) {
#ifdef USE_EPSILON
        if (fabs(a.s[i] - b.s[i]) > EPSILON)
            return false;
#else
        if (a.s[i] != b.s[i])
            return false;
#endif
    }
    return true;
}

bool operator==(const VolumeType& a, const VolumeType& b) {
    for (auto i = 0u; i != sizeof(a) / sizeof(a.s[0]); ++i) {
#ifdef USE_EPSILON
        if (fabs(a.s[i] - b.s[i]) > EPSILON)
            return false;
#else
        if (a.s[i] != b.s[i])
            return false;
#endif
    }
    return true;
}
*/

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
    constexpr auto v = 0.9;
    constexpr Surface surface{VolumeType{{v, v, v, v, v, v, v, v}},
                              VolumeType{{v, v, v, v, v, v, v, v}}};

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
                    auto base_vol = pow(-v, reflections);
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

    raytracer::attenuator::microphone attenuator(cc.get_context(),
                                                 cc.get_device());
    auto output = attenuator.process(results->get_impulses(true, true, false),
                                     glm::vec3(0, 0, 1),
                                     0,
                                     receiver);

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
        auto bit_depth   = 16;
        auto sample_rate = 44100.0;
        aligned::vector<aligned::vector<aligned::vector<float>>> flattened{
                raytracer::flatten_impulses(i, sample_rate)};
        {
            auto mixed_down = mixdown(flattened);
            normalize(mixed_down);
            snd::write(
                    build_string(SCRATCH_PATH, "/", name, "_no_processing.wav"),
                    mixed_down,
                    sample_rate,
                    bit_depth);
        }
        {
            auto processed = raytracer::process(
                    flattened, sample_rate, false, 1, true, 1);
            normalize(processed);
            snd::write(build_string(SCRATCH_PATH, "/", name, "_processed.wav"),
                       processed,
                       sample_rate,
                       bit_depth);
        }
    };

    postprocess(output, "raytraced");
    postprocess(proper_image_source_impulses, "image_src");
}
