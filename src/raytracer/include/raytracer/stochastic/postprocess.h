#pragma once

#include "raytracer/stochastic/postprocessing.h"

namespace wayverb {
namespace raytracer {
namespace stochastic {

struct size_functor final {
    template <typename T>
    auto operator()(const T& t) const {
        return t.size();
    }
};

template <size_t Az, size_t El, typename Method>
auto postprocess(const directional_energy_histogram<Az, El>& histogram,
                 const Method& method,
                 double room_volume,
                 const core::environment& environment,
                 double sample_rate) {
    const auto& table = histogram.histogram.table;

    const auto max_size = std::accumulate(
            std::begin(table),
            std::end(table),
            0ul,
            [&](auto a, const auto& b) {
                if (b.empty()) {
                    return a;
                }
                const auto make_size_iterator = [](auto it) {
                    return util::make_mapping_iterator_adapter(std::move(it),
                                                               size_functor{});
                };
                return std::max(
                        a,
                        *std::max_element(make_size_iterator(std::begin(b)),
                                          make_size_iterator(std::end(b))));
            });

    const auto max_seconds = max_size / histogram.sample_rate;

    const auto dirac_sequence = generate_dirac_sequence(
            environment.speed_of_sound, room_volume, sample_rate, max_seconds);
    return postprocessing(
            histogram, method, dirac_sequence, environment.acoustic_impedance);
}

}  // namespace stochastic
}  // namespace raytracer
}  // namespace wayverb
