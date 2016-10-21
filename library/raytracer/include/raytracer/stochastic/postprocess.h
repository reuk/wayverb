#pragma once

#include "common/model/receiver.h"
#include "raytracer/stochastic/postprocessing.h"

namespace raytracer {
namespace stochastic {

template <typename Method>
auto postprocess(const energy_histogram& histogram,
                 const Method& method,
                 const glm::vec3& position,
                 double room_volume,
                 double acoustic_impedance,
                 double speed_of_sound,
                 double sample_rate,
                 double max_seconds) {
    //  TODO should be directional depending on receiver...
    //  throw std::runtime_error{"stochastic::postprocess not implemented yet"};

    const auto dirac_sequence = generate_dirac_sequence(
            speed_of_sound, room_volume, sample_rate, max_seconds);
    return mono_postprocessing(histogram, dirac_sequence, acoustic_impedance);
}

}  // namespace stochastic
}  // namespace raytracer
