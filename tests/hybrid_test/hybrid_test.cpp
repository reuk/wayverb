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

auto run_raytracer(const ContextInfo& context_info,
                   const CuboidBoundary& boundary,
                   const CombinedConfig& config,
                   const std::string& output_folder) {
    auto raytrace_program =
        get_program<RayverbProgram>(context_info.context, context_info.device);

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
    auto output = attenuator.attenuate(results.get_all(false), {speaker});
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

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    CHECK(argc == 2) << "expected an output folder";

    auto output_folder = argv[1];

    //  init OpenCL
    auto context = get_context();
    auto device = get_device(context);

    CHECK(device.getInfo<CL_DEVICE_AVAILABLE>())
        << "opencl device is not available!";

    auto queue = cl::CommandQueue(context, device);

    //  init simulation parameters
    CuboidBoundary boundary(Vec3f(0, 0, 0), Vec3f(4, 3, 6));
    LOG(INFO) << "boundary: " << boundary;

    CombinedConfig config;
    config.get_filter_frequency() = 1000;
    config.get_source() = Vec3f(2, 1, 1);
    config.get_mic() = Vec3f(2, 1, 5);

    auto context_info = ContextInfo{context, device, queue};

    auto waveguide_output =
        run_waveguide(context_info, boundary, config, output_folder);

    auto raytracer_output =
        run_raytracer(context_info, boundary, config, output_folder);
}
