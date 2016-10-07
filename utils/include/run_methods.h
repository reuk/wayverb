#pragma once

/// This file contains methods for running acoustic simulations in boxes with
/// a single type of 'flat-response' surface.

#include "waveguide/attenuator.h"
#include "waveguide/calibration.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocessor/directional_receiver.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/surface_filters.h"
#include "waveguide/waveguide.h"

#include "raytracer/image_source/exact.h"
#include "raytracer/image_source/postprocessors.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "common/reverb_time.h"

#include "utilities/progress_bar.h"

#include <iostream>

template <typename It>
auto waveguide_filter(It begin, It end, float sample_rate) {
    //  Filter waveguide output.
    const auto waveguide_max{0.16};
    const auto normalised_width{0.02};
    const auto normalised_cutoff{waveguide_max - (normalised_width / 2)};

    frequency_domain::filter filter{
            static_cast<size_t>(std::distance(begin, end)) * 2};
    filter.run(begin, end, begin, [=](auto cplx, auto freq) {
        const auto ret{
                cplx *
                static_cast<float>(frequency_domain::compute_lopass_magnitude(
                        freq, normalised_cutoff, normalised_width, 0))};
        const auto hipass{false};
        if (hipass) {
            const auto low_cutoff{100 / sample_rate};
            return ret * static_cast<float>(
                                 frequency_domain::compute_hipass_magnitude(
                                         freq, low_cutoff, low_cutoff * 2, 0));
        }
        return ret;
    });
}

auto run_waveguide(const geo::box& box,
                   float absorption,
                   const glm::vec3& source,
                   const glm::vec3& receiver,
                   const microphone& microphone,
                   float speed_of_sound,
                   float acoustic_impedance,
                   float sample_rate,
                   float simulation_time) {
    std::cout << "running waveguide\n";

    const compute_context cc{};
    auto voxels_and_mesh{waveguide::compute_voxels_and_mesh(
            cc,
            geo::get_scene_data(box, make_surface(absorption, 0)),
            source,
            sample_rate,
            speed_of_sound)};

    auto& mesh{std::get<1>(voxels_and_mesh)};
    mesh.set_coefficients(
            {waveguide::to_flat_coefficients(make_surface(absorption, 0))});

    const auto input_node{compute_index(mesh.get_descriptor(), source)};
    const auto output_node{compute_index(mesh.get_descriptor(), receiver)};

    const auto grid_spacing{mesh.get_descriptor().spacing};

    const auto calibration_factor{waveguide::rectilinear_calibration_factor(
            grid_spacing, acoustic_impedance)};
    std::cout << "calibration factor: " << calibration_factor << '\n';

    aligned::vector<float> input_signal{static_cast<float>(calibration_factor)};
    input_signal.resize(simulation_time * sample_rate, 0.0f);

    std::cout << "running " << input_signal.size() << " steps\n";

    auto prep{waveguide::preprocessor::make_hard_source(
            input_node, input_signal.begin(), input_signal.end())};

    callback_accumulator<waveguide::postprocessor::directional_receiver> post{
            mesh.get_descriptor(),
            sample_rate,
            acoustic_impedance / speed_of_sound,
            output_node};

    progress_bar pb{std::cerr, input_signal.size()};
    waveguide::run(cc,
                   mesh,
                   prep,
                   [&](auto& queue, const auto& buffer, auto step) {
                       post(queue, buffer, step);
                       pb += 1;
                   },
                   true);

    return post.get_output();
}

template <typename It>
auto postprocess_impulses(It begin,
                          It end,
                          const glm::vec3& receiver,
                          const microphone& microphone,
                          float speed_of_sound,
                          float acoustic_impedance,
                          float sample_rate) {
    //  Correct for directionality of the receiver.
    auto attenuated{raytracer::attenuate(microphone, receiver, begin, end)};

    //  Correct for distance travelled.
    for (auto& it : attenuated) {
        it.volume *= pressure_for_distance(it.distance, acoustic_impedance);
    }

    //  Mix down to histogram.
    const auto histogram{raytracer::sinc_histogram(attenuated.begin(),
                                                   attenuated.end(),
                                                   speed_of_sound,
                                                   sample_rate,
                                                   20)};

    //  Extract.
    return map_to_vector(std::begin(histogram),
                         std::end(histogram),
                         [](auto i) { return i.s[0]; });
}

auto run_exact_img_src(const geo::box& box,
                       float absorption,
                       const glm::vec3& source,
                       const glm::vec3& receiver,
                       const microphone& microphone,
                       float speed_of_sound,
                       float acoustic_impedance,
                       float sample_rate,
                       float simulation_time) {
    std::cout << "running exact img src\n";

    auto impulses{raytracer::image_source::find_impulses<
            raytracer::image_source::fast_pressure_calculator<cl_float1>>(
            box,
            source,
            receiver,
            cl_float1{{absorption}},
            simulation_time * speed_of_sound)};

    return postprocess_impulses(impulses.begin(),
                                impulses.end(),
                                receiver,
                                microphone,
                                speed_of_sound,
                                acoustic_impedance,
                                sample_rate);
}

auto run_fast_img_src(const geo::box& box,
                      float absorption,
                      const glm::vec3& source,
                      const glm::vec3& receiver,
                      const microphone& microphone,
                      float speed_of_sound,
                      float acoustic_impedance,
                      float sample_rate,
                      float) {
    std::cout << "running fast img src\n";

    const auto voxelised{make_voxelised_scene_data(
            geo::get_scene_data(box, make_surface(absorption, 0)), 2, 0.1f)};

    const auto directions{get_random_directions(1 << 13)};
    auto impulses{raytracer::image_source::run<
            raytracer::image_source::fast_pressure_calculator<surface>>(
            directions.begin(),
            directions.end(),
            compute_context{},
            voxelised,
            source,
            receiver)};

    if (const auto direct{raytracer::get_direct(source, receiver, voxelised)}) {
        impulses.emplace_back(*direct);
    }

    return postprocess_impulses(impulses.begin(),
                                impulses.end(),
                                receiver,
                                microphone,
                                speed_of_sound,
                                acoustic_impedance,
                                sample_rate);
}

template <typename Callback>
auto run(const geo::box& box,
         float absorption,
         const glm::vec3& source,
         const glm::vec3& receiver,
         const microphone& microphone,
         float speed_of_sound,
         float acoustic_impedance,
         float sample_rate,
         const Callback& callback) {
    const auto eyring{
            eyring_reverb_time(geo::get_scene_data(box, absorption), 0)};
    std::cout << "expected reverb time: " << eyring << '\n';
    return callback(box,
                    absorption,
                    source,
                    receiver,
                    microphone,
                    speed_of_sound,
                    acoustic_impedance,
                    sample_rate,
                    eyring);
}

