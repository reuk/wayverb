#pragma once

#include "raytracer/image_source/postprocess.h"
#include "raytracer/stochastic/postprocess.h"

namespace raytracer {

template <typename Method>
auto postprocess(const std::tuple<aligned::vector<impulse<simulation_bands>>,
                                  energy_histogram>& input,
                 const Method& method,
                 const glm::vec3& position,
                 double room_volume,
                 double acoustic_impedance,
                 double speed_of_sound,
                 double sample_rate,
                 double max_time) {
    const auto head = raytracer::image_source::postprocess(begin(get<0>(input)),
                                                           end(get<0>(input)),
                                                           method,
                                                           position,
                                                           speed_of_sound,
                                                           sample_rate,
                                                           max_time);
    const auto tail = raytracer::stochastic::postprocess(get<1>(input),
                                                         method,
                                                         position,
                                                         room_volume,
                                                         acoustic_impedance,
                                                         speed_of_sound,
                                                         sample_rate,
                                                         max_time);
    return sum_vectors(head, tail);
}

}  // namesapce raytracer
