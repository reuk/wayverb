#pragma once

#include "waveguide/calibration.h"
#include "waveguide/fitted_boundary.h"
#include "waveguide/mesh.h"
#include "waveguide/postprocess.h"
#include "waveguide/postprocessor/directional_receiver.h"
#include "waveguide/preprocessor/hard_source.h"
#include "waveguide/waveguide.h"

#include "common/geo/box.h"
#include "common/model/parameters.h"

#include "utilities/aligned/vector.h"
#include "utilities/progress_bar.h"

template <typename It>
void lopass(It b, It e, double sample_rate) {
    const auto waveguide_max{0.16};
    const auto normalised_width{0.02};
    const auto normalised_cutoff{waveguide_max - (normalised_width / 2)};

    frequency_domain::filter filter{static_cast<size_t>(std::distance(b, e)) *
                                    2};
    filter.run(b, e, b, [=](auto cplx, auto freq) {
        const auto ret{
                cplx *
                static_cast<float>(frequency_domain::compute_lopass_magnitude(
                        freq,
                        frequency_domain::edge_and_width{normalised_cutoff,
                                                         normalised_width}))};
        const auto hipass{false};
        if (hipass) {
            const auto low_cutoff{100 / sample_rate};
            return ret * static_cast<float>(
                                 frequency_domain::compute_hipass_magnitude(
                                         freq,
                                         frequency_domain::edge_and_width{
                                                 low_cutoff, low_cutoff * 2}));
        }
        return ret;
    });
}

template <typename It>
auto postprocess_waveguide(It b, It e, double sample_rate) {
    auto processed{waveguide::postprocess(b, e, sample_rate)};
    lopass(begin(processed), end(processed), sample_rate);
    return processed;
}

template <typename It, typename Attenuator>
auto postprocess_waveguide(It b,
                           It e,
                           const Attenuator& attenuator,
                           float sample_rate,
                           float acoustic_impedance) {
    auto processed{waveguide::postprocess(
            b, e, attenuator, acoustic_impedance, sample_rate)};
    lopass(begin(processed), end(processed), sample_rate);
    return processed;
}

template <typename Absorption>
aligned::vector<waveguide::postprocessor::directional_receiver::output>
run_waveguide(const geo::box& box,
              Absorption absorption,
              const model::parameters& params,
              float sample_rate,
              float simulation_time) {
    const compute_context cc{};
    auto voxels_and_mesh{waveguide::compute_voxels_and_mesh(
            cc,
            geo::get_scene_data(box, absorption),
            params.source,
            sample_rate,
            params.speed_of_sound)};

    //  TODO stop using flat coefficients probably

    voxels_and_mesh.mesh.set_coefficients(
            {waveguide::to_flat_coefficients(absorption)});

    const auto input_node{compute_index(voxels_and_mesh.mesh.get_descriptor(),
                                        params.source)};
    const auto output_node{compute_index(voxels_and_mesh.mesh.get_descriptor(),
                                         params.receiver)};

    const auto grid_spacing{voxels_and_mesh.mesh.get_descriptor().spacing};

    const auto calibration_factor{waveguide::rectilinear_calibration_factor(
            grid_spacing, params.acoustic_impedance)};

    std::cerr << "calibration factor: " << calibration_factor << '\n';

    aligned::vector<float> input_signal{static_cast<float>(calibration_factor)};
    input_signal.resize(simulation_time * sample_rate, 0.0f);

    auto prep{waveguide::preprocessor::make_hard_source(
            input_node, input_signal.begin(), input_signal.end())};

    callback_accumulator<waveguide::postprocessor::directional_receiver> post{
            voxels_and_mesh.mesh.get_descriptor(),
            sample_rate,
            params.acoustic_impedance / params.speed_of_sound,
            output_node};

    progress_bar pb;
    run(cc,
        voxels_and_mesh.mesh,
        prep,
        [&](auto& queue, const auto& buffer, auto step) {
            post(queue, buffer, step);
            set_progress(pb, step, input_signal.size());
        },
        true);

    return post.get_output();
}
