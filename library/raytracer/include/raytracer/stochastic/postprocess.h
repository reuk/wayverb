#pragma once

#include "raytracer/stochastic/postprocessing.h"

#include "common/model/receiver.h"

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
                 const glm::vec3& position,
                 double room_volume,
                 double acoustic_impedance,
                 double speed_of_sound,
                 double sample_rate) {
    const auto& table = histogram.histogram.table;

    const auto max_size = std::accumulate(
            std::begin(table),
            std::end(table),
            0ul,
            [&](auto a, const auto& b) {
                const auto make_size_iterator = [](auto it) {
                    return make_mapping_iterator_adapter(std::move(it),
                                                         size_functor{});
                };
                return std::max(
                        a,
                        *std::max_element(make_size_iterator(std::begin(b)),
                                          make_size_iterator(std::end(b))));
            });

    const auto max_seconds = max_size / histogram.sample_rate;

    const auto dirac_sequence = generate_dirac_sequence(
            speed_of_sound, room_volume, sample_rate, max_seconds);
    return postprocessing(
            histogram, method, position, dirac_sequence, acoustic_impedance);
}

}  // namespace stochastic
}  // namespace raytracer
