#pragma once

#include "waveguide/canonical.h"
#include "waveguide/postprocess.h"

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
                        freq, normalised_cutoff, normalised_width))};
        const auto hipass{false};
        if (hipass) {
            const auto low_cutoff{100 / sample_rate};
            return ret * static_cast<float>(
                                 frequency_domain::compute_hipass_magnitude(
                                         freq, low_cutoff, low_cutoff * 2));
        }
        return ret;
    });
}

template <typename Attenuator>
auto postprocess_waveguide(const waveguide::simulation_results& sim_results,
                           const Attenuator& attenuator,
                           float sample_rate,
                           float acoustic_impedance) {
    auto processed{waveguide::postprocess(
            sim_results, attenuator, acoustic_impedance, sample_rate)};
    lopass(begin(processed), end(processed), sample_rate);
    return processed;
}

template <typename Absorption>
auto run_waveguide(const geo::box& box,
                   Absorption absorption,
                   const model::parameters& params,
                   float sample_rate,
                   float simulation_time) {
    const auto scene = geo::get_scene_data(box, absorption);
    progress_bar pb;
    return *waveguide::canonical(
            compute_context{},
            scene,
            params,
            waveguide::single_band_parameters{sample_rate, 0.6},
            max_element(eyring_reverb_time(scene, 0.0)),
            true,
            [&](auto step, auto steps) { set_progress(pb, step, steps); });
}
