#include "combined_config.h"

#include "azimuth_elevation.h"
#include "conversions.h"
#include "microphone.h"
#include "scene_data.h"
#include "test_flag.h"
#include "waveguide.h"

#include "raytracer.h"

#include "cl_common.h"

#include "stl_wrappers.h"

//  dependency
#include "filters_common.h"
#include "sinc.h"
#include "write_audio_file.h"

#include "samplerate.h"
#include "sndfile.hh"

#include <gflags/gflags.h>

#include <glog/logging.h>

//  stdlib
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

/// courant number is 1 / sqrt(3) for a rectilinear mesh
/// but sqrt isn't constexpr >:(
constexpr auto COURANT = 0.577350269;

/// r = distance at which the geometric sound source has intensity 1W/m^2
/// sr = waveguide mesh sampling rate
constexpr double rectilinear_calibration_factor(double r, double sr) {
    auto x = SPEED_OF_SOUND / (COURANT * sr);
    return r / (x * 0.3405);
}

void write_file(const config::App& config,
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

auto run_waveguide(ComputeContext& context_info,
                   const CuboidBoundary& boundary,
                   const config::Combined& config,
                   const std::string& output_folder) {
    auto steps = 16000;

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
    proc::transform(
        results, output.begin(), [](const auto& i) { return i.pressure; });

    return output;
}

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    constexpr Box box(Vec3f(0, 0, 0), Vec3f(4, 3, 6));
    constexpr Vec3f source(1, 1, 1);
    constexpr Vec3f receiver(2, 1, 5);
    constexpr auto samplerate = 44100;
    constexpr auto v = 1;
    constexpr Surface surface{{{v, v, v, v, v, v, v, v}},
                              {{v, v, v, v, v, v, v, v}}};

    CHECK(argc == 2) << "expected an output folder";

    auto output_folder = argv[1];

    //  init simulation parameters
    CuboidBoundary boundary(box.get_c0(), box.get_c1(), {surface});
    LOG(INFO) << "boundary: " << boundary;

    config::Combined config;
    config.get_filter_frequency() = 1000;
    config.get_source() = source;
    config.get_mic() = receiver;
    config.get_output_sample_rate() = samplerate;

    ComputeContext context_info;

    auto waveguide_output =
        run_waveguide(context_info, boundary, config, output_folder);

    std::cout << "max mag: " << max_mag(waveguide_output) << std::endl;
    write_file(config, output_folder, "waveguide_raw", {waveguide_output});

    //  get the valid region of the spectrum
    filter::LinkwitzRileyLopass lopass;
    lopass.setParams(config.get_filter_frequency(),
                     config.get_waveguide_sample_rate());
    lopass.filter(waveguide_output);

    //  adjust sample rate
    auto waveguide_adjusted = adjust_sampling_rate(waveguide_output, config);

    std::cout << "max mag: " << max_mag(waveguide_adjusted) << std::endl;
    write_file(
        config, output_folder, "waveguide_adjusted", {waveguide_adjusted});

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
        attenuator.attenuate(results.get_image_source(false), {speaker})
            .front();

    std::vector<std::vector<std::vector<float>>> flattened = {
        flatten_impulses(output, config.get_output_sample_rate())};

    write_file(
        config, output_folder, "raytrace_no_processing", mixdown(flattened));

    auto raytracer_output = process(filter::FilterType::linkwitz_riley,
                                    flattened,
                                    config.get_output_sample_rate(),
                                    false,
                                    1,
                                    true,
                                    1)
                                .front();

    auto waveguide_copy = waveguide_adjusted;
    normalize(waveguide_copy);
    write_file(config, output_folder, "waveguide_normalized", {waveguide_copy});

    auto raytracer_copy = raytracer_output;
    normalize(raytracer_copy);
    write_file(
        config, output_folder, "raytracer_normalized", {raytracer_output});

    mul(waveguide_adjusted,
        rectilinear_calibration_factor(1, config.get_waveguide_sample_rate()));

    auto max_waveguide = max_mag(waveguide_adjusted);
    auto max_raytracer = max_mag(raytracer_output);
    auto max_both = std::max(max_waveguide, max_raytracer);
    std::cout << "max amplitude between both methods: " << max_both
              << std::endl;

    auto waveguide_length = waveguide_adjusted.size();
    auto raytracer_length = raytracer_output.size();
    auto out_length = std::min(waveguide_length, raytracer_length);

    waveguide_adjusted.resize(out_length);
    raytracer_output.resize(out_length);

    mul(waveguide_adjusted, 1 / max_both);
    mul(raytracer_output, 1 / max_both);

    auto window = right_hanning(out_length);
    elementwise_multiply(waveguide_adjusted, window);
    elementwise_multiply(raytracer_output, window);

    write_file(
        config, output_folder, "waveguide_processed", {waveguide_adjusted});
    write_file(
        config, output_folder, "raytracer_processed", {raytracer_output});
}
