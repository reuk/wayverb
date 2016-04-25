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

#include "boundaries_serialize.h"
#include "surface_owner_serialize.h"

#include "combined_config_serialize.h"
#include "json_read_write.h"

//  dependency
#include "filters_common.h"
#include "sinc.h"
#include "write_audio_file.h"

#include "samplerate.h"
#include "sndfile.hh"

#include "gflags/gflags.h"

#include "glog/logging.h"

//  stdlib
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <random>

namespace std {
template <typename T>
JSON_OSTREAM_OVERLOAD(vector<T>);
}  // namespace std

/// courant number is 1 / sqrt(3) for a rectilinear mesh
/// but sqrt isn't constexpr >:(
constexpr auto COURANT = 0.577350269;

/// given a source strength, calculate the distance at which the source produces
/// intensity 1W/m^2
double distance_for_unit_intensity(double strength) {
    return std::sqrt(strength / (4 * M_PI));
}

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
    auto depth = get_file_depth(config.bit_depth);

    write_sndfile(output_file, output, config.sample_rate, depth, format);
}

auto run_waveguide(ComputeContext& context_info,
                   const CuboidBoundary& boundary,
                   const config::Combined& config,
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
                                   config.mic,
                                   config.get_waveguide_sample_rate());

    auto source_index = waveguide.get_index_for_coordinate(config.source);
    auto receiver_index = waveguide.get_index_for_coordinate(config.mic);

    CHECK(waveguide.inside(source_index)) << "source is outside of mesh!";
    CHECK(waveguide.inside(receiver_index)) << "receiver is outside of mesh!";

    auto corrected_source = waveguide.get_coordinate_for_index(source_index);

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

    constexpr Box box(Vec3f(0, 0, 0), Vec3f(5.56, 3.97, 2.81));
    constexpr Vec3f source(2.09, 2.12, 2.12);
    constexpr Vec3f receiver(2.09, 3.08, 0.96);
    constexpr auto samplerate = 96000;
    constexpr auto v = 0.9;
    constexpr Surface surface{{{v, v, v, v, v, v, v, v}},
                              {{v, v, v, v, v, v, v, v}}};

    CHECK(argc == 2) << "expected an output folder";

    std::string output_folder = argv[1];

    //  init simulation parameters
    CuboidBoundary boundary(box.get_c0(), box.get_c1(), {surface});

    config::Combined config;
    config.filter_frequency = 2000;
    config.source = source;
    config.mic = receiver;
    config.sample_rate = samplerate;

    json_read_write::write(output_folder + "/used_config.json", config);

    ComputeContext context_info;

    auto waveguide_output =
        run_waveguide(context_info, boundary, config, output_folder);
    LOG(INFO) << "waveguide output mag: " << max_mag(waveguide_output);

    write_file(config, output_folder, "waveguide_raw", {waveguide_output});

    //  adjust sample rate
    auto waveguide_adjusted = adjust_sampling_rate(waveguide_output, config);
    LOG(INFO) << "waveguide adjusted mag: " << max_mag(waveguide_adjusted);

    //  get the valid region of the spectrum
    filter::LopassWindowedSinc lopass(waveguide_adjusted.size());
    lopass.set_params(config.filter_frequency, config.sample_rate);
    lopass.filter(waveguide_adjusted);
    LOG(INFO) << "waveguide filtered mag: " << max_mag(waveguide_adjusted);

    write_file(
        config, output_folder, "waveguide_adjusted", {waveguide_adjusted});

    auto raytrace_program = get_program<RaytracerProgram>(context_info.context,
                                                          context_info.device);

    Raytracer raytracer(raytrace_program, context_info.queue);
    //            [                                        ]
    std::cout << "[ -- running raytracer ----------------- ]" << std::endl;
    ProgressBar pb(std::cout, config.impulses);
    auto results = raytracer.run(boundary.get_scene_data(),
                                 config.mic,
                                 config.source,
                                 config.rays,
                                 config.impulses,
                                 [&pb] { pb += 1; });

    Attenuate attenuator(raytrace_program, context_info.queue);
    Speaker speaker{};
    auto output =
        attenuator.attenuate(results.get_image_source(false), {speaker})
            .front();
    // auto output =
    //    attenuator.attenuate(results.get_all(false), {speaker}).front();

    std::vector<std::vector<std::vector<float>>> flattened = {
        flatten_impulses(output, config.sample_rate)};

    write_file(
        config, output_folder, "raytrace_no_processing", mixdown(flattened));

    filter::run(
        filter::FilterType::linkwitz_riley, flattened, config.sample_rate, 1);
    auto raytracer_output = mixdown(flattened).front();

    auto write_normalized = [&config, &output_folder](auto i, auto name) {
        normalize(i);
        write_file(config, output_folder, name, {i});
    };

    write_normalized(waveguide_adjusted, "waveguide_normalized");
    write_normalized(raytracer_output, "raytracer_normalized");

    auto calibration_factor = rectilinear_calibration_factor(
        distance_for_unit_intensity(1), config.get_waveguide_sample_rate());
    LOG(INFO) << "calibration factor: " << calibration_factor;

    mul(waveguide_adjusted, calibration_factor);

    auto waveguide_length = waveguide_adjusted.size();
    auto raytracer_length = raytracer_output.size();
    auto out_length = std::min(waveguide_length, raytracer_length);

    waveguide_adjusted.resize(out_length);
    raytracer_output.resize(out_length);

    auto max_waveguide = max_mag(waveguide_adjusted);
    auto max_raytracer = max_mag(raytracer_output);
    auto max_both = std::max(max_waveguide, max_raytracer);
    LOG(INFO) << "max waveguide: " << max_waveguide;
    LOG(INFO) << "max raytracer: " << max_raytracer;

    mul(waveguide_adjusted, 1 / max_both);
    mul(raytracer_output, 1 / max_both);

    auto window = right_hanning(out_length);
    elementwise_multiply(waveguide_adjusted, window);
    elementwise_multiply(raytracer_output, window);

    write_file(
        config, output_folder, "waveguide_processed", {waveguide_adjusted});
    write_file(
        config, output_folder, "raytracer_processed", {raytracer_output});

    filter::HipassWindowedSinc hipass(raytracer_output.size());
    hipass.set_params(config.filter_frequency, config.sample_rate);
    hipass.filter(raytracer_output);
    LOG(INFO) << "max raytracer filtered: " << max_mag(raytracer_output);

    std::vector<float> mixed(out_length);
    proc::transform(waveguide_adjusted,
                    raytracer_output.begin(),
                    mixed.begin(),
                    [](auto a, auto b) { return a + b; });

    write_file(config, output_folder, "mixed", {mixed});
}
