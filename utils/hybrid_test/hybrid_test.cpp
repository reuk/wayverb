#include "combined/model.h"
#include "combined/serialize/model.h"

#include "waveguide/attenuator/microphone.h"
#include "waveguide/config.h"
#include "waveguide/mesh/boundary_adjust.h"
#include "waveguide/mesh/model.h"
#include "waveguide/waveguide.h"

#include "raytracer/attenuator.h"
#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflector.h"

#include "common/azimuth_elevation.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/dc_blocker.h"
#include "common/filters_common.h"
#include "common/hrtf_utils.h"
#include "common/kernel.h"
#include "common/progress_bar.h"
#include "common/scene_data.h"
#include "common/sinc.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/stl_wrappers.h"
#include "common/write_audio_file.h"

#include "common/serialize/json_read_write.h"
#include "common/serialize/surface.h"

#include "samplerate.h"
#include "sndfile.hh"

#include "gflags/gflags.h"

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

constexpr auto speed_of_sound{340.0};
constexpr auto acoustic_impedance{400.0};

/// courant number is 1 / sqrt(3) for a rectilinear mesh
/// but sqrt isn't constexpr >:(
constexpr auto courant = 0.577350269;

/// given a source strength, calculate the distance at which the source produces
/// intensity 1W/m^2
double distance_for_unit_intensity(double strength) {
    return std::sqrt(strength / (4 * M_PI));
}

/// r = distance at which the geometric sound source has intensity 1W/m^2
/// sr = waveguide mesh sampling rate
constexpr double rectilinear_calibration_factor(double r, double sr) {
    const auto x = speed_of_sound / (courant * sr);
    return r / (x * 0.3405);
}

auto run_waveguide(const compute_context& cc,
                   const voxelised_scene_data& boundary,
                   const model::SingleShot& config,
                   const std::string& output_folder,
                   float spacing) {
    const auto steps = 16000;

    //  get a waveguide
    const auto receiver = config.receiver_settings.position;

    const auto model{waveguide::mesh::compute_model(
            cc, boundary, spacing, speed_of_sound)};

    const auto receiver_index = compute_index(model.get_descriptor(), receiver);
    const auto source_index =
            compute_index(model.get_descriptor(), config.source);

    if (!waveguide::mesh::is_inside(model, receiver_index)) {
        throw std::runtime_error("receiver is outside of mesh!");
    }
    if (!waveguide::mesh::is_inside(model, source_index)) {
        throw std::runtime_error("source is outside of mesh!");
    }

    auto input = kernels::sin_modulated_gaussian_kernel(
            get_waveguide_sample_rate(config));
    input.resize(steps);

    //  run the waveguide
    //            [                                        ]
    std::cout << "[ -- running waveguide ----------------- ]" << std::endl;
    progress_bar pb(std::cout, steps);
    const auto results{waveguide::run(cc,
                                      model,
                                      source_index,
                                      input,
                                      receiver_index,
                                      speed_of_sound,
                                      acoustic_impedance,
                                      [&](auto) { pb += 1; })};

    auto output = map_to_vector(results, [](auto i) { return i.pressure; });

    //  correct for filter time offset
    output.erase(output.begin(),
                 output.begin() + std::ceil(input.size() / 2.0));

    return output;
}

int main(int argc, char** argv) {
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    geo::box box(glm::vec3(0, 0, 0), glm::vec3(5.56, 3.97, 2.81));
    constexpr glm::vec3 source(2.09, 2.12, 2.12);
    constexpr glm::vec3 receiver(2.09, 3.08, 0.96);
    constexpr auto samplerate = 96000;
    constexpr auto bit_depth = 16;
    constexpr auto v = 0.9;
    constexpr surface surface{volume_type{{v, v, v, v, v, v, v, v}},
                              volume_type{{v, v, v, v, v, v, v, v}}};

    auto scene = geo::get_scene_data(box);
    scene.set_surfaces(surface);

    const model::SingleShot config{
            2000, 2, 340, 100000, source, model::ReceiverSettings{receiver}};

    const auto spacing = waveguide::config::grid_spacing(
            speed_of_sound, 1 / get_waveguide_sample_rate(config));

    const voxelised_scene_data voxelised(
            scene,
            5,
            waveguide::compute_adjusted_boundary(
                    scene.get_aabb(), receiver, spacing));

    if (argc != 2) {
        throw std::runtime_error("expected an output folder");
    }

    std::string output_folder = argv[1];

    //  init simulation parameters

    json_read_write::write(output_folder + "/used_config.json", config);

    compute_context cc;

    auto waveguide_output =
            run_waveguide(cc, voxelised, config, output_folder, spacing);

    // filter::ZeroPhaseDCBlocker(32).filter(waveguide_output);

    snd::write(build_string(output_folder, "/waveguide_raw.wav"),
               {waveguide_output},
               samplerate,
               bit_depth);

    //  adjust sample rate
    auto waveguide_adjusted =
            waveguide::adjust_sampling_rate(std::move(waveguide_output),
                                            get_waveguide_sample_rate(config),
                                            samplerate);
    std::cerr << "waveguide adjusted mag: " << max_mag(waveguide_adjusted)
              << '\n';

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
    //            [                                        ]
    std::cout << "[ -- running raytracer ----------------- ]" << std::endl;
    std::atomic_bool keep_going{true};
    const auto impulses = 1000;
    progress_bar pb(std::cout, impulses);
    const auto results{raytracer::run(cc,
                                      voxelised,
                                      speed_of_sound,
                                      config.source,
                                      config.receiver_settings.position,
                                      get_random_directions(config.rays),
                                      impulses,
                                      10,
                                      keep_going,
                                      [&](auto) { pb += 1; })};

    raytracer::attenuator::microphone attenuator{cc, speed_of_sound};
    const auto output{
            attenuator.process(results->get_impulses(true, true, false),
                               glm::vec3(0, 0, 1),
                               0,
                               receiver)};
    // auto output =
    //    attenuator.attenuate(results.get_all(false), {speaker}).front();

    aligned::vector<aligned::vector<float>> flattened =
            raytracer::flatten_impulses(output, samplerate, acoustic_impedance);

    snd::write(build_string(output_folder, "raytrace_no_processing.wav"),
               {mixdown(flattened)},
               samplerate,
               bit_depth);

    auto raytracer_output = multiband_filter_and_mixdown(flattened, samplerate);

    auto write_normalized = [bit_depth, samplerate, &output_folder](auto i,
                                                                    auto name) {
        normalize(i);
        snd::write(build_string(output_folder, "/", name, ".wav"),
                   {i},
                   samplerate,
                   bit_depth);
    };

    write_normalized(waveguide_adjusted, "waveguide_normalized");
    write_normalized(raytracer_output, "raytracer_normalized");

    auto calibration_factor = rectilinear_calibration_factor(
            distance_for_unit_intensity(1), get_waveguide_sample_rate(config));
    std::cerr << "calibration factor: " << calibration_factor << '\n';

    mul(waveguide_adjusted, calibration_factor);

    auto waveguide_length = waveguide_adjusted.size();
    auto raytracer_length = raytracer_output.size();
    auto out_length = std::min(waveguide_length, raytracer_length);

    waveguide_adjusted.resize(out_length);
    raytracer_output.resize(out_length);

    auto max_waveguide = max_mag(waveguide_adjusted);
    auto max_raytracer = max_mag(raytracer_output);
    auto max_both = std::max(max_waveguide, max_raytracer);
    std::cerr << "max waveguide: " << max_waveguide << '\n';
    std::cerr << "max raytracer: " << max_raytracer << '\n';

    mul(waveguide_adjusted, 1 / max_both);
    mul(raytracer_output, 1 / max_both);

    auto window = right_hanning(out_length);
    elementwise_multiply(waveguide_adjusted, window);
    elementwise_multiply(raytracer_output, window);

    snd::write(build_string(output_folder, "/waveguide_processed.wav"),
               {waveguide_adjusted},
               samplerate,
               bit_depth);
    snd::write(build_string(output_folder, "/raytracer_processed.wav"),
               {raytracer_output},
               samplerate,
               bit_depth);

    filter::HipassWindowedSinc hipass(raytracer_output.size());
    hipass.set_params(config.filter_frequency, samplerate);
    raytracer_output =
            hipass.filter(raytracer_output.begin(), raytracer_output.end());
    std::cerr << "max raytracer filtered: " << max_mag(raytracer_output)
              << '\n';

    aligned::vector<float> mixed(out_length);
    proc::transform(waveguide_adjusted,
                    raytracer_output.begin(),
                    mixed.begin(),
                    [](auto a, auto b) { return a + b; });

    snd::write(build_string(output_folder, "/mixed.wav"),
               {mixed},
               samplerate,
               bit_depth);
}
