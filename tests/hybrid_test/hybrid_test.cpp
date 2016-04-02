#include "combined_config.h"

#include "waveguide.h"
#include "scene_data.h"
#include "test_flag.h"
#include "conversions.h"
#include "microphone.h"
#include "azimuth_elevation.h"

#include "raytracer.h"

#include "cl_common.h"

//  dependency
#include "filters_common.h"
#include "sinc.h"
#include "write_audio_file.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include "sndfile.hh"
#include "samplerate.h"

#include <gflags/gflags.h>

#include <glog/logging.h>

//  stdlib
#include <random>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <map>
#include <iomanip>

/// courant number is 1 / sqrt(3) for a rectilinear mesh
/// but sqrt isn't constexpr >:(
constexpr auto COURANT = 0.577350269;

/// r = distance at which the geometric sound source has intensity 1W/m^2
/// sr = waveguide mesh sampling rate
/// c = speed of sound
constexpr double rectilinear_calibration_factor(double r, double sr, double c) {
    auto x = COURANT * sr / c;
    return r / (x * 0.3405);
}

void write_file(const Config& config,
                const std::string& output_folder,
                const std::string& fname,
                const std::vector<std::vector<float>>& output) {
    auto output_file = build_string(output_folder, "/", fname, ".wav");
    LOG(INFO) << "writing file: " << output_file;

    auto format = get_file_format(output_file);
    auto depth = get_file_depth(config.get_bit_depth());

    write_sndfile(
        output_file, output, config.get_output_sample_rate(), depth, format);
}

struct ContextInfo {
    cl::Context context;
    cl::Device device;
    cl::CommandQueue& queue;
};

auto run_waveguide(const ContextInfo& context_info,
                   const CuboidBoundary& boundary,
                   const CombinedConfig& config,
                   const std::string& output_folder) {
    auto steps = 4000;

    //  get opencl program
    auto waveguide_program = get_program<RectangularProgram>(
        context_info.context, context_info.device);

    //  get a waveguide
    RectangularWaveguide waveguide(waveguide_program,
                                   context_info.queue,
                                   boundary,
                                   config.get_divisions(),
                                   config.get_mic(),
                                   config.get_waveguide_sample_rate());

    auto source_index = waveguide.get_index_for_coordinate(config.get_source());
    auto receiver_index = waveguide.get_index_for_coordinate(config.get_mic());

    CHECK(waveguide.inside(source_index)) << "source is outside of mesh!";
    CHECK(waveguide.inside(receiver_index)) << "receiver is outside of mesh!";

    auto corrected_source = waveguide.get_coordinate_for_index(source_index);
    auto corrected_receiver =
        waveguide.get_coordinate_for_index(receiver_index);

    LOG(INFO) << "running simulation!";
    LOG(INFO) << "source pos: " << corrected_source;
    LOG(INFO) << "mic pos: " << corrected_receiver;

    //  run the waveguide
    //            [                                        ]
    std::cout << "[ -- running waveguide ----------------- ]" << std::endl;
    ProgressBar pb(std::cout, steps);
    auto results = waveguide.run_basic(corrected_source,
                                       receiver_index,
                                       steps,
                                       config.get_waveguide_sample_rate(),
                                       [&pb] { pb += 1; });

    auto output = std::vector<float>(results.size());
    std::transform(results.begin(),
                   results.end(),
                   output.begin(),
                   [](const auto& i) { return i.pressure; });

    //  get the valid region of the spectrum
    LinkwitzRileyLopass lopass;
    lopass.setParams(config.get_filter_frequency(),
                     config.get_waveguide_sample_rate());
    lopass.filter(output);

    DCBlocker dc_blocker;
    dc_blocker.filter(output);

    //  adjust sample rate
    auto adjusted = adjust_sampling_rate(output, config);
    auto ret = adjusted;
    normalize(adjusted);
    write_file(
        config, output_folder, "waveguide_filtered_normalized", {adjusted});
    return ret;
}

struct Box {
    enum class Wall {
        nx,
        px,
        ny,
        py,
        nz,
        pz,
    };

    enum class Direction {
        x,
        y,
        z,
    };

    constexpr Box(const Vec3f& c0, const Vec3f& c1)
            : Box(std::array<Vec3f, 2>{{c0, c1}}) {
    }

    constexpr Box(const std::array<Vec3f, 2>& v)
            : c0(get_min(v))
            , c1(get_max(v)) {
    }

    constexpr bool inside(const Vec3f& v) const {
        return (c0 < v).all() && (v < c1).all();
    }

    constexpr Vec3f centre() const {
        return (c0 + c1) * 0.5;
    }

    constexpr Vec3f mirror_on_axis(const Vec3f& v,
                                   const Vec3f& pt,
                                   Direction d) const {
        switch (d) {
            case Direction::x:
                return Vec3f(2 * pt.x - v.x, v.y, v.z);
            case Direction::y:
                return Vec3f(v.x, 2 * pt.y - v.y, v.z);
            case Direction::z:
                return Vec3f(v.x, v.y, 2 * pt.z - v.z);
        }
    }

    constexpr Vec3f mirror_inside(const Vec3f& v, Direction d) const {
        return mirror_on_axis(v, centre(), d);
    }

    constexpr Vec3f mirror(const Vec3f& v, Wall w) const {
        switch (w) {
            case Wall::nx:
                return mirror_on_axis(v, c0, Direction::x);
            case Wall::px:
                return mirror_on_axis(v, c1, Direction::x);
            case Wall::ny:
                return mirror_on_axis(v, c0, Direction::y);
            case Wall::py:
                return mirror_on_axis(v, c1, Direction::y);
            case Wall::nz:
                return mirror_on_axis(v, c0, Direction::z);
            case Wall::pz:
                return mirror_on_axis(v, c1, Direction::z);
        }
    }

    constexpr Box mirror(Wall w) const {
        return Box(mirror(c0, w), mirror(c1, w));
    }

    constexpr bool operator==(const Box& b) const {
        return (c0 == b.c0).all() && (c1 == b.c1).all();
    }

    constexpr Box operator+(const Vec3f& v) const {
        return Box(c0 + v, c1 + v);
    }

    constexpr Box operator-(const Vec3f& v) const {
        return Box(c0 - v, c1 - v);
    }

    constexpr Vec3f dimensions() const {
        return c1 - c0;
    }

    Vec3f c0, c1;
};

constexpr Box test_box(Vec3f(0), Vec3f(2, 4, 6));
static_assert((test_box.mirror_inside(Vec3f(0.5, 1, 1), Box::Direction::x) ==
               Vec3f(1.5, 1, 1))
                  .all(),
              "box fail");
static_assert((test_box.mirror_inside(Vec3f(0.5, 1, 1), Box::Direction::y) ==
               Vec3f(0.5, 3, 1))
                  .all(),
              "box fail");
static_assert((test_box.mirror_inside(Vec3f(0.5, 1, 1), Box::Direction::z) ==
               Vec3f(0.5, 1, 5))
                  .all(),
              "box fail");

auto run_raytracer(const ContextInfo& context_info,
                   const CuboidBoundary& boundary,
                   const CombinedConfig& config,
                   const std::string& output_folder) {
    auto raytrace_program = get_program<RaytracerProgram>(context_info.context,
                                                          context_info.device);

    Raytracer raytracer(raytrace_program, context_info.queue);
    //            [                                        ]
    std::cout << "[ -- running raytracer ----------------- ]" << std::endl;
    ProgressBar pb(std::cout, config.get_impulses());
    auto results = raytracer.run(boundary.get_scene_data(),
                                 config.get_mic(),
                                 config.get_source(),
                                 config.get_rays(),
                                 config.get_impulses(),
                                 [&pb] { pb += 1; });

    Attenuate attenuator(raytrace_program, context_info.queue);
    Speaker speaker{};
    auto output =
        attenuator.attenuate(results.get_image_source(false), {speaker});
    // auto output = attenuator.attenuate(results.get_all(false), {speaker});
    auto flattened = flatten_impulses(output, config.get_output_sample_rate());
    auto processed = process(FilterType::FILTER_TYPE_LINKWITZ_RILEY,
                             flattened,
                             config.get_output_sample_rate(),
                             false,
                             1,
                             true,
                             1);

    //  get the valid region of the spectrum
    LinkwitzRileyHipass hipass;
    hipass.setParams(config.get_filter_frequency(),
                     config.get_output_sample_rate());
    for (auto& i : processed)
        hipass.filter(i);

    auto ret = processed;
    normalize(processed);
    write_file(
        config, output_folder, "raytrace_filtered_normalized", processed);
    return ret;
}

constexpr int outer_boxes_2d(int n) {
    if (n == 0)
        return 1;
    return n * 4;
}

static_assert(outer_boxes_2d(1) == 4, "bad outer");
static_assert(outer_boxes_2d(2) == 8, "bad outer");
static_assert(outer_boxes_2d(3) == 12, "bad outer");

constexpr int outer_box_factor_3d(int n) {
    if (n == 0)
        return 1;
    return n == 1 ? 1 : outer_boxes_2d(n - 1) + outer_box_factor_3d(n - 1);
}

static_assert(outer_box_factor_3d(1) == 1, "bad factor");
static_assert(outer_box_factor_3d(2) == 5, "bad factor");
static_assert(outer_box_factor_3d(3) == 13, "bad factor");

constexpr int outer_boxes_3d(int n) {
    return outer_box_factor_3d(n) * 2 + outer_boxes_2d(n);
}

static_assert(outer_boxes_3d(1) == 6, "bad outer");
static_assert(outer_boxes_3d(2) == 18, "bad outer");
static_assert(outer_boxes_3d(3) == 38, "bad outer");
static_assert(outer_boxes_3d(4) == 66, "bad outer");

constexpr Box box(Vec3f(0, 0, 0), Vec3f(4, 3, 6));
constexpr Vec3f source(1, 1, 1);
constexpr Vec3f receiver(2, 1, 5);
constexpr auto samplerate = 44100;

constexpr int pow(int x, int p) {
    return p == 0 ? 1 : x * pow(x, p - 1);
}

constexpr int width_for_shell(int shell) {
    return shell * 2 + 1;
}

constexpr int num_images(int shell) {
    return pow(width_for_shell(shell), 3);
}

template <int SHELL>
std::array<Vec3f, num_images(SHELL)> images_for_shell() {
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

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    constexpr auto shells = 8;
    auto images = images_for_shell<shells>();
    std::array<float, images.size()> times;
    std::transform(images.begin(),
                   images.end(),
                   times.begin(),
                   [](auto i) { return (receiver - i).mag() / 340; });

    auto max_time = *std::max_element(times.begin(), times.end());
    std::vector<float> proper_image_source(max_time * samplerate + 1, 0);

    constexpr auto L = width_for_shell(shells);
    for (int i = 0; i != L; ++i) {
        for (int j = 0; j != L; ++j) {
            for (int k = 0; k != L; ++k) {
                auto reflections = Vec3f(std::abs(i - shells),
                                         std::abs(j - shells),
                                         std::abs(k - shells))
                                       .sum();
                proper_image_source[times[i + j * L + k * L * L] *
                                    samplerate] += pow(0.8, reflections);
            }
        }
    }
    normalize(proper_image_source);

    CHECK(argc == 2) << "expected an output folder";

    auto output_folder = argv[1];

    //  init OpenCL
    auto context = get_context();
    auto device = get_device(context);

    CHECK(device.getInfo<CL_DEVICE_AVAILABLE>())
        << "opencl device is not available!";

    auto queue = cl::CommandQueue(context, device);

    //  init simulation parameters
    CuboidBoundary boundary(box.c0, box.c1);
    LOG(INFO) << "boundary: " << boundary;

    CombinedConfig config;
    config.get_filter_frequency() = 1000;
    config.get_source() = source;
    config.get_mic() = receiver;
    config.get_output_sample_rate() = samplerate;

    write_file(
        config, output_folder, "proper_image_source", {proper_image_source});

    auto context_info = ContextInfo{context, device, queue};

    auto waveguide_output =
        run_waveguide(context_info, boundary, config, output_folder);

    write_file(config, output_folder, "waveguide_filtered", {waveguide_output});

    auto raytracer_output =
        run_raytracer(context_info, boundary, config, output_folder);

    write_file(config, output_folder, "raytracer_filtered", raytracer_output);
}
