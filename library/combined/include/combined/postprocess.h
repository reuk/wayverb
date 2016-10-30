#pragma once

#include "raytracer/postprocess.h"

#include "waveguide/config.h"
#include "waveguide/postprocess.h"

namespace wayverb {

struct waveguide_results final {
    aligned::vector<waveguide::postprocessor::directional_receiver::output>
            directional;
    double sample_rate;
};

template <typename Histogram>
struct combined_results final {
    raytracer::aural_results<Histogram> raytracer;
    waveguide_results waveguide;
};

template <typename Histogram>
auto make_combined_results(raytracer::aural_results<Histogram> raytracer,
                           waveguide_results waveguide) {
    return combined_results<Histogram>{std::move(raytracer),
                                       std::move(waveguide)};
}

////////////////////////////////////////////////////////////////////////////////

template <typename LoIt, typename HiIt>
auto crossover_filter(LoIt b_lo,
                      LoIt e_lo,
                      HiIt b_hi,
                      HiIt e_hi,
                      double cutoff,
                      double width) {
    frequency_domain::filter filt{
            frequency_domain::best_fft_length(std::max(
                    std::distance(b_lo, e_lo), std::distance(b_hi, e_hi)))
            << 2};

    constexpr auto l = 0;

    return sum_vectors(
            [&] {
                auto ret = std::vector<float>(std::distance(b_lo, e_lo));
                filt.run(b_lo, e_lo, begin(ret), [&](auto cplx, auto freq) {
                    return cplx *
                           compute_lopass_magnitude(freq, cutoff, width, l);
                });
                return ret;
            }(),
            [&] {
                auto ret = std::vector<float>(std::distance(b_hi, e_hi));
                filt.run(b_hi, e_hi, begin(ret), [&](auto cplx, auto freq) {
                    return cplx *
                           compute_hipass_magnitude(freq, cutoff, width, l);
                });
            }());
}

////////////////////////////////////////////////////////////////////////////////

template <typename Histogram, typename Method>
auto postprocess(const combined_results<Histogram>& input,
                 const Method& method,
                 const glm::vec3& receiver_position,
                 double room_volume,
                 double acoustic_impedance,
                 double speed_of_sound,
                 double output_sample_rate) {
    //  Individual processing.
    const auto waveguide_processed = [&] {
        auto ret = waveguide::postprocess(begin(input.waveguide.directional),
                                          end(input.waveguide.directional),
                                          method,
                                          acoustic_impedance,
                                          input.waveguide.sample_rate);
        //  TODO DC removal.

        //  Samplerate conversion.
        ret = waveguide::adjust_sampling_rate(ret.data(),
                                              ret.data() + ret.size(),
                                              input.waveguide.sample_rate,
                                              output_sample_rate);

        return ret;
    }();

    const auto raytracer_processed = raytracer::postprocess(input.raytracer,
                                                            method,
                                                            receiver_position,
                                                            room_volume,
                                                            acoustic_impedance,
                                                            speed_of_sound,
                                                            output_sample_rate);

    return crossover_filter(begin(waveguide_processed),
                            end(waveguide_processed),
                            begin(raytracer_processed),
                            end(raytracer_processed),
                            cutoff,
                            width);
}

}  // namespace wayverb
