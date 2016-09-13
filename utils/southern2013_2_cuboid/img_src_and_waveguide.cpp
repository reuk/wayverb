#include "img_src_and_waveguide.h"

#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "waveguide/calibration.h"
#include "waveguide/make_transparent.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "common/azimuth_elevation.h"
#include "common/frequency_domain_filter.h"
#include "common/progress_bar.h"

#include <iostream>

//  current problems
//      * waveguide contribution is way too short
//
//  needs testing
//      * modal response / level
//      * ricker input
//      * microphone processing

img_src_and_waveguide_test::img_src_and_waveguide_test(const scene_data& sd,
                                                       float speed_of_sound,
                                                       float acoustic_impedance)
        : voxels_and_mesh_{waveguide::compute_voxels_and_mesh(
                  compute_context_,
                  sd,
                  glm::vec3{0},
                  waveguide_sample_rate_,
                  speed_of_sound)}
        , speed_of_sound_{speed_of_sound}
        , acoustic_impedance_{acoustic_impedance} {}

audio img_src_and_waveguide_test::operator()(
        const surface& surface,
        const glm::vec3& source,
        const model::receiver_settings& receiver) {
    auto& voxels{std::get<0>(voxels_and_mesh_)};
    voxels.set_surfaces(surface);

    auto& mesh{std::get<1>(voxels_and_mesh_)};
    mesh.set_coefficients(waveguide::to_filter_coefficients(
            voxels.get_scene_data().get_surfaces(), waveguide_sample_rate_));

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{
            compute_index(mesh.get_descriptor(), receiver.position)};

    const aligned::vector<float> dry_signal{1};
    const auto input_signal{waveguide::make_transparent(dry_signal)};

    progress_bar pb{std::cout, input_signal.size()};
    const auto waveguide_results{waveguide::run(compute_context_,
                                                mesh,
                                                input_node,
                                                input_signal,
                                                output_node,
                                                speed_of_sound_,
                                                acoustic_impedance_,
                                                [&](auto i) { pb += 1; })};

    const float calibration_factor = waveguide::rectilinear_calibration_factor(
            waveguide_sample_rate_, speed_of_sound_);
    const auto sample_rate{44100.0};
    auto corrected_waveguide{waveguide::adjust_sampling_rate(
            map_to_vector(
                    waveguide_results,
                    [=](auto i) { return i.pressure * calibration_factor; }),
            waveguide_sample_rate_,
            sample_rate)};

    const auto directions{get_random_directions(100000)};

    auto impulses{raytracer::image_source::run<
            raytracer::image_source::fast_pressure_calculator>(
            directions.begin(),
            directions.end(),
            compute_context_,
            voxels,
            source,
            receiver.position,
            speed_of_sound_,
            acoustic_impedance_,
            sample_rate)};

    if (const auto direct{raytracer::get_direct_impulse(
                source, receiver.position, voxels, speed_of_sound_)}) {
        impulses.emplace_back(*direct);
    }

    const auto img_src_results{
            mixdown(raytracer::convert_to_histogram(impulses.begin(),
                                                    impulses.end(),
                                                    sample_rate,
                                                    acoustic_impedance_,
                                                    20))};

    auto pressure{map_to_vector(img_src_results,
                                [](auto i) { return std::sqrt(i); })};

    const auto max_size{std::max(corrected_waveguide.size(), pressure.size())};
    {
        //  Do some crossover filtering
        const auto cutoff{waveguide_sample_rate_ * 0.196 / sample_rate};
        const auto width{std::min(cutoff, 0.5 - cutoff) * 0.2};
        pressure.resize(max_size);
        corrected_waveguide.resize(max_size);
        fast_filter filter{max_size * 2};
        filter.filter(pressure.begin(),
                      pressure.end(),
                      pressure.begin(),
                      [=](auto cplx, auto freq) {
                          return cplx * compute_hipass_magnitude(
                                                freq, cutoff, width, 0);
                      });
        filter.filter(corrected_waveguide.begin(),
                      corrected_waveguide.end(),
                      corrected_waveguide.begin(),
                      [=](auto cplx, auto freq) {
                          return cplx * compute_lopass_magnitude(
                                                freq, cutoff, width, 0);
                      });
    }

    aligned::vector<float> output(max_size);

    for (auto i{0ul}, end{pressure.size()}; i != end; ++i) {
        output[i] += pressure[i];
    }

    for (auto i{0ul}, end{corrected_waveguide.size()}; i != end; ++i) {
        output[i] += corrected_waveguide[i];
    }

    return {std::move(output), sample_rate, "img_src_and_waveguide"};
}
