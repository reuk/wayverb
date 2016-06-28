#include "combined/model.h"

#include "waveguide/microphone.h"
#include "waveguide/config.h"
#include "waveguide/waveguide.h"

#include "raytracer/raytracer.h"

#include "common/azimuth_elevation.h"
#include "common/boundaries_serialize.h"
#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/dc_blocker.h"
#include "common/filters_common.h"
#include "common/json_read_write.h"
#include "common/kernel.h"
#include "common/scene_data.h"
#include "common/sinc.h"
#include "common/stl_wrappers.h"
#include "common/surface_serialize.h"
#include "common/write_audio_file.h"

#include "samplerate.h"
#include "sndfile.hh"

#include "gflags/gflags.h"

#include "glog/logging.h"

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

void write_file(size_t bit_depth,
                double sample_rate,
                const std::string& output_folder,
                const std::string& fname,
                const std::vector<std::vector<float>>& output) {
    auto output_file = build_string(output_folder, "/", fname, ".wav");
    LOG(INFO) << "writing file: " << output_file;

    auto format = get_file_format(output_file);
    auto depth = get_file_depth(bit_depth);

    write_sndfile(output_file, output, sample_rate, depth, format);
}

auto run_waveguide(ComputeContext& context_info,
                   const CuboidBoundary& boundary,
                   const model::App& config,
                   const std::string& output_folder,
                   const Surface& surface) {
    auto steps = 16000;

    //  get opencl program
    auto waveguide_program = get_program<RectangularProgram>(
            context_info.context, context_info.device);

    auto scene_data = boundary.get_scene_data();
    scene_data.set_surfaces(surface);

    //  get a waveguide
    RectangularWaveguide<BufferType::cl> waveguide(
            waveguide_program,
            context_info.queue,
            MeshBoundary(scene_data),
            config.receiver,
            config.get_waveguide_sample_rate());

    auto source_index = waveguide.get_index_for_coordinate(config.source);
    auto receiver_index = waveguide.get_index_for_coordinate(config.receiver);

    CHECK(waveguide.inside(source_index)) << "source is outside of mesh!";
    CHECK(waveguide.inside(receiver_index)) << "receiver is outside of mesh!";

    auto corrected_source = waveguide.get_coordinate_for_index(source_index);

    auto input = waveguide_kernel(config.get_waveguide_sample_rate());

    LOG(INFO) << input;

    //  run the waveguide
    //            [                                        ]
    std::cout << "[ -- running waveguide ----------------- ]" << std::endl;
    std::atomic_bool keep_going{true};
    ProgressBar pb(std::cout, steps);
    auto results = waveguide.init_and_run(corrected_source,
                                          std::move(input),
                                          receiver_index,
                                          steps,
                                          keep_going,
                                          [&pb] { pb += 1; });

    auto output = std::vector<float>(results.size());
    proc::transform(
            results, output.begin(), [](const auto& i) { return i.pressure; });

    //  correct for filter time offset
    output.erase(output.begin(),
                 output.begin() + std::ceil(input.size() / 2.0));

    return output;
}

int main(int argc, char** argv) {
    google::InitGoogleLogging(argv[0]);
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    Box box(glm::vec3(0, 0, 0), glm::vec3(5.56, 3.97, 2.81));
    constexpr glm::vec3 source(2.09, 2.12, 2.12);
    constexpr glm::vec3 receiver(2.09, 3.08, 0.96);
    constexpr auto samplerate = 96000;
    constexpr auto bit_depth = 16;
    constexpr auto v = 0.9;
    constexpr Surface surface{{{v, v, v, v, v, v, v, v}},
                              {{v, v, v, v, v, v, v, v}}};

    CHECK(argc == 2) << "expected an output folder";

    std::string output_folder = argv[1];

    //  init simulation parameters
    CuboidBoundary boundary(box.get_c0(), box.get_c1());

    const model::App config{2000, 2, 100000, source, receiver};

    json_read_write::write(output_folder + "/used_config.json", config);

    ComputeContext context_info;

    auto waveguide_output = run_waveguide(
            context_info, boundary, config, output_folder, surface);

    // filter::ZeroPhaseDCBlocker(32).filter(waveguide_output);

    write_file(bit_depth,
               samplerate,
               output_folder,
               "waveguide_raw",
               {waveguide_output});

    //  adjust sample rate
    auto waveguide_adjusted =
            adjust_sampling_rate(std::move(waveguide_output),
                                 config.get_waveguide_sample_rate(),
                                 samplerate);
    LOG(INFO) << "waveguide adjusted mag: " << max_mag(waveguide_adjusted);

    //  get the valid region of the spectrum
    //    filter::LopassWindowedSinc lopass(waveguide_adjusted.size());
    //    lopass.set_params(config.filter_frequency, config.sample_rate);
    //    lopass.filter(waveguide_adjusted);
    //    LOG(INFO) << "waveguide filtered mag: " <<
    //    max_mag(waveguide_adjusted);
    //
    //    write_file(
    //        config, output_folder, "waveguide_adjusted",
    //        {waveguide_adjusted});
    //
    auto raytrace_program = get_program<RaytracerProgram>(context_info.context,
                                                          context_info.device);

    Raytracer raytracer(raytrace_program, context_info.queue);
    //            [                                        ]
    std::cout << "[ -- running raytracer ----------------- ]" << std::endl;
    std::atomic_bool keep_going{true};
    const auto impulses = 1000;
    ProgressBar pb(std::cout, impulses);
    auto results = raytracer.run(boundary.get_scene_data(),
                                 config.receiver,
                                 config.source,
                                 config.rays,
                                 impulses,
                                 keep_going,
                                 [&pb] { pb += 1; });

    Attenuate attenuator(raytrace_program, context_info.queue);
    Speaker speaker{};
    auto output =
            attenuator.attenuate(results.get_image_source(false), {speaker})
                    .front();
    // auto output =
    //    attenuator.attenuate(results.get_all(false), {speaker}).front();

    std::vector<std::vector<std::vector<float>>> flattened = {
            flatten_impulses(output, samplerate)};

    write_file(bit_depth,
               samplerate,
               output_folder,
               "raytrace_no_processing",
               mixdown(flattened));

    filter::run(filter::FilterType::linkwitz_riley,
                flattened,
                samplerate,
                1);
    auto raytracer_output = mixdown(flattened).front();

    auto write_normalized = [bit_depth, samplerate, &output_folder](auto i,
                                                                    auto name) {
        normalize(i);
        write_file(bit_depth, samplerate, output_folder, name, {i});
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

    write_file(bit_depth,
               samplerate,
               output_folder,
               "waveguide_processed",
               {waveguide_adjusted});
    write_file(bit_depth,
               samplerate,
               output_folder,
               "raytracer_processed",
               {raytracer_output});

    filter::HipassWindowedSinc hipass(raytracer_output.size());
    hipass.set_params(config.filter_frequency, samplerate);
    hipass.filter(raytracer_output);
    LOG(INFO) << "max raytracer filtered: " << max_mag(raytracer_output);

    std::vector<float> mixed(out_length);
    proc::transform(waveguide_adjusted,
                    raytracer_output.begin(),
                    mixed.begin(),
                    [](auto a, auto b) { return a + b; });

    write_file(bit_depth, samplerate, output_folder, "mixed", {mixed});
}
