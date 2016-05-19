#include "raytracer/config.h"
#include "raytracer/raytracer.h"

#include "common/boundaries.h"
#include "common/cl_common.h"
#include "common/progress.h"
#include "common/write_audio_file.h"

#include "glog/logging.h"
#include "gtest/gtest.h"

#include <set>

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef MAT_PATH
#define MAT_PATH ""
#endif

#ifndef SCRATCH_PATH
#define SCRATCH_PATH ""
#endif

template <typename T, typename Prog>
auto get_results(ComputeContext& context,
                 const Prog& prog,
                 const SceneData& scene_data,
                 const std::vector<cl_float3>& directions,
                 int reflections) {
    std::atomic_bool keep_going{true};
    T raytrace(prog, context.queue);
    return raytrace.run(scene_data,
                        Vec3f(0, 1.75, 0),
                        Vec3f(0, 1.75, 3),
                        directions,
                        reflections,
                        keep_going);
}

void write_file(const config::App& config,
                const std::string& fname,
                const std::vector<std::vector<float>>& output) {
    auto output_file = build_string(SCRATCH_PATH, "/", fname, ".wav");
    LOG(INFO) << "writing file: " << output_file;

    auto format = get_file_format(output_file);
    auto depth = get_file_depth(config.bit_depth);

    write_sndfile(output_file, output, config.sample_rate, depth, format);
}

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

bool operator==(const Impulse& a, const Impulse& b) {
    if (!(a.volume == b.volume))
        return false;
    if (!(a.position == b.position))
        return false;
    if (!(a.time == b.time))
        return false;

    return true;
}

static constexpr auto bench_reflections = 128;
static constexpr auto bench_rays = 1 << 15;

TEST(raytrace, new) {
    ComputeContext context;
    auto raytrace_program = get_program<RaytracerProgram>(context);

    SceneData scene_data(OBJ_PATH, MAT_PATH);

    auto directions = get_random_directions(bench_rays);

    auto results_1 = get_results<Raytracer>(
        context, raytrace_program, scene_data, directions, bench_reflections);
}

constexpr int width_for_shell(int shell) {
    return shell * 2 + 1;
}

constexpr int num_images(int shell) {
    auto w = width_for_shell(shell);
    return w * w * w;
}

template <int SHELL>
std::array<Vec3f, num_images(SHELL)> images_for_shell(const Box& box,
                                                      const Vec3f& source) {
    std::array<Vec3f, num_images(SHELL)> ret;

    auto image = source;

    constexpr auto L = width_for_shell(SHELL);
    for (int i = 0; i != L; ++i) {
        auto x = i - SHELL;
        auto reflected_x =
            x % 2 ? box.mirror_inside(image, Box::Direction::x) : image;
        for (int j = 0; j != L; ++j) {
            auto y = j - SHELL;
            auto reflected_y =
                y % 2 ? box.mirror_inside(reflected_x, Box::Direction::y)
                      : reflected_x;
            for (int k = 0; k != L; ++k) {
                auto z = k - SHELL;
                auto reflected_z =
                    z % 2 ? box.mirror_inside(reflected_y, Box::Direction::z)
                          : reflected_y;

                ret[i + j * L + k * L * L] =
                    reflected_z + Vec3f(x, y, z) * box.dimensions();
            }
        }
    }

    return ret;
}

TEST(raytrace, image_source) {
    /*
    auto log_attenuation = [](auto i) {
        LOG(INFO) << "attenuation for distance: " << i << " = "
                  << attenuation_for_distance(i);
    };

    log_attenuation(0);
    log_attenuation(1);
    log_attenuation(2);
    log_attenuation(10);
    */

    //  proper method
    constexpr Box box(Vec3f(0, 0, 0), Vec3f(4, 3, 6));
    constexpr Vec3f source(1, 1, 1);
    constexpr Vec3f receiver(2, 1, 5);
    constexpr auto samplerate = 44100;
    constexpr auto v = 0.9;
    constexpr Surface surface{{{v, v, v, v, v, v, v, v}},
                              {{v, v, v, v, v, v, v, v}}};

    constexpr auto shells = 2;
    auto images = images_for_shell<shells>(box, source);
    std::array<float, images.size()> distances;
    proc::transform(images, distances.begin(), [&receiver](auto i) {
        return (receiver - i).mag();
    });
    std::array<float, images.size()> times;
    proc::transform(distances, times.begin(), [](auto i) { return i / 340; });
    std::array<VolumeType, images.size()> volumes;
    proc::transform(distances, volumes.begin(), [](auto i) {
        return attenuation_for_distance(i);
    });

    std::vector<AttenuatedImpulse> proper_image_source_impulses;

    constexpr auto L = width_for_shell(shells);
    for (int i = 0; i != L; ++i) {
        for (int j = 0; j != L; ++j) {
            for (int k = 0; k != L; ++k) {
                auto reflections = Vec3f(std::abs(i - shells),
                                         std::abs(j - shells),
                                         std::abs(k - shells))
                                       .sum();

                if (reflections <= shells) {
                    auto index = i + j * L + k * L * L;
                    auto volume = volumes[index];
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
    ComputeContext context;
    auto raytrace_program = get_program<RaytracerProgram>(context);

    CuboidBoundary boundary(box.get_c0(), box.get_c1());

    config::Raytracer config;
    config.source = source;
    config.mic = receiver;
    config.sample_rate = samplerate;

    Raytracer raytracer(raytrace_program, context.queue);

    auto scene_data = boundary.get_scene_data();
    scene_data.set_surfaces(surface);

    std::atomic_bool keep_going{true};
    auto results = raytracer.run(scene_data,
                                 config.mic,
                                 config.source,
                                 config.rays,
                                 config.impulses,
                                 keep_going);

    Attenuate attenuator(raytrace_program, context.queue);
    Speaker speaker{};
    auto output =
        attenuator.attenuate(results.get_image_source(false), {speaker})
            .front();

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
        ASSERT_TRUE(std::abs(i.time - closest.time) < 0.001) << i.time << " "
                                                             << closest.time;
    }

    auto postprocess = [&config](const auto& i, const std::string& name) {
        std::vector<std::vector<std::vector<float>>> flattened = {
            flatten_impulses(i, config.sample_rate)};
        {
            auto mixed_down = mixdown(flattened);
            normalize(mixed_down);
            write_file(config, name + "_no_processing", mixed_down);
        }
        {
            auto processed = process(filter::FilterType::linkwitz_riley,
                                     flattened,
                                     config.sample_rate,
                                     false,
                                     1,
                                     true,
                                     1);
            normalize(processed);
            write_file(config, name + "_processed", processed);
        }
    };

    postprocess(output, "raytraced");
    postprocess(proper_image_source_impulses, "image_src");
}
