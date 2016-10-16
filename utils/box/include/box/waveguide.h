#pragma once

#include "waveguide/postprocess.h"
#include "waveguide/postprocessor/directional_receiver.h"

#include "common/geo/box.h"
#include "common/model/parameters.h"

#include "utilities/aligned/vector.h"

template <typename It>
void lopass(It b, It e, double sample_rate) {
    const auto waveguide_max{0.16};
    const auto normalised_width{0.02};
    const auto normalised_cutoff{waveguide_max - (normalised_width / 2)};

    frequency_domain::filter filter{static_cast<size_t>(std::distance(b, e)) *
                                    2};
    filter.run(
            b, e, b, [=](auto cplx, auto freq) {
                const auto ret{
                        cplx *
                        static_cast<float>(
                                frequency_domain::compute_lopass_magnitude(
                                        freq,
                                        frequency_domain::edge_and_width{
                                                normalised_cutoff,
                                                normalised_width}))};
                const auto hipass{false};
                if (hipass) {
                    const auto low_cutoff{100 / sample_rate};
                    return ret *
                           static_cast<float>(
                                   frequency_domain::compute_hipass_magnitude(
                                           freq,
                                           frequency_domain::edge_and_width{
                                                   low_cutoff,
                                                   low_cutoff * 2}));
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

aligned::vector<waveguide::postprocessor::directional_receiver::output>
run_waveguide(const geo::box& box,
              float absorption,
              const model::parameters& params,
              float sample_rate,
              float simulation_time);
