#pragma once

#include "raytracer/postprocess.h"

#include "waveguide/canonical.h"
#include "waveguide/config.h"
#include "waveguide/postprocess.h"

#include "core/sum_ranges.h"

namespace wayverb {

template <typename Histogram>
struct combined_results final {
    raytracer::simulation_results<Histogram> raytracer;
    waveguide::simulation_results waveguide;
};

template <typename Histogram>
auto make_combined_results(raytracer::simulation_results<Histogram> raytracer,
                           waveguide::simulation_results waveguide) {
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

    const auto run_filter = [&](auto b, auto e, auto mag_func) {
        auto ret = std::vector<float>(std::distance(b, e));
        filt.run(b, e, begin(ret), [&](auto cplx, auto freq) {
            return cplx * static_cast<float>(mag_func(freq, cutoff, width, l));
        });
        return ret;
    };

    return sum_vectors(
            run_filter(b_lo, e_lo, frequency_domain::compute_lopass_magnitude),
            run_filter(b_hi, e_hi, frequency_domain::compute_hipass_magnitude));
}

////////////////////////////////////////////////////////////////////////////////

struct max_frequency_functor final {
    template <typename T>
    auto operator()(T&& t) const {
        return t.valid_hz.get_max();
    }
};

template <typename Histogram, typename Method>
auto postprocess(const combined_results<Histogram>& input,
                 const Method& method,
                 const glm::vec3& receiver_position,
                 double room_volume,
                 double acoustic_impedance,
                 double speed_of_sound,
                 double output_sample_rate) {
    //  Individual processing.
    const auto waveguide_processed = waveguide::postprocess(
            input.waveguide, method, acoustic_impedance, output_sample_rate);

    const auto raytracer_processed = raytracer::postprocess(input.raytracer,
                                                            method,
                                                            receiver_position,
                                                            room_volume,
                                                            acoustic_impedance,
                                                            speed_of_sound,
                                                            output_sample_rate);

    const auto make_iterator = [](auto it) {
        return util::make_mapping_iterator_adapter(std::move(it),
                                                   max_frequency_functor{});
    };

    const auto cutoff =
            *std::max_element(make_iterator(begin(input.waveguide.bands)),
                              make_iterator(end(input.waveguide.bands))) /
            output_sample_rate;
    const auto width = 0.2;  //  Wider = more natural-sounding
    return crossover_filter(begin(waveguide_processed),
                            end(waveguide_processed),
                            begin(raytracer_processed),
                            end(raytracer_processed),
                            cutoff,
                            width);
}

}  // namespace wayverb
