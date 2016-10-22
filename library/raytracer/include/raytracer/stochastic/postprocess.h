#pragma once

#include "common/model/receiver.h"
#include "raytracer/stochastic/postprocessing.h"

namespace raytracer {
namespace stochastic {

template <size_t Az, size_t El, typename Method>
auto postprocess(const directional_energy_histogram<Az, El>& histogram,
                 const Method& method,
                 const glm::vec3& position,
                 double room_volume,
                 double acoustic_impedance,
                 double speed_of_sound,
                 double sample_rate,
                 double max_seconds) {
    const auto dirac_sequence = generate_dirac_sequence(
            speed_of_sound, room_volume, sample_rate, max_seconds);
    return postprocessing(
            histogram, method, position, dirac_sequence, acoustic_impedance);
}

}  // namespace stochastic
}  // namespace raytracer
