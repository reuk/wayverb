#pragma once

#include "waveguide/attenuator.h"
#include "waveguide/multiband.h"
#include "waveguide/postprocessor/directional_receiver.h"

#include "common/geo/box.h"

#include "utilities/aligned/vector.h"

template <typename It, typename Attenuator>
auto postprocess_waveguide(It begin,
                           It end,
                           const Attenuator& attenuator,
                           float sample_rate,
                           float acoustic_impedance) {
    auto attenuated{
            waveguide::attenuate(attenuator, acoustic_impedance, begin, end)};

    auto processed{waveguide::multiband_process(
            std::begin(attenuated), std::end(attenuated), sample_rate)};

    //  Filter waveguide output.
    const auto waveguide_max{0.16};
    const auto normalised_width{0.02};
    const auto normalised_cutoff{waveguide_max - (normalised_width / 2)};

    frequency_domain::filter filter{processed.size() * 2};
    filter.run(
            processed.begin(),
            processed.end(),
            processed.begin(),
            [=](auto cplx, auto freq) {
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
    return processed;
}

aligned::vector<waveguide::postprocessor::directional_receiver::output>
run_waveguide(const geo::box& box,
              float absorption,
              const glm::vec3& source,
              const glm::vec3& receiver,
              float speed_of_sound,
              float acoustic_impedance,
              float sample_rate,
              float simulation_time);
