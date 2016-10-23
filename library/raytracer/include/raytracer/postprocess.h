#pragma once

#include "raytracer/image_source/postprocess.h"
#include "raytracer/stochastic/postprocess.h"

#include "common/sum_ranges.h"

namespace raytracer {

template <typename Histogram, typename Method>
auto postprocess(const std::tuple<aligned::vector<impulse<simulation_bands>>,
                                  Histogram>& input,
                 const Method& method,
                 const glm::vec3& position,
                 double room_volume,
                 double acoustic_impedance,
                 double speed_of_sound,
                 double sample_rate) {
    const auto head =
            raytracer::image_source::postprocess(begin(std::get<0>(input)),
                                                 end(std::get<0>(input)),
                                                 method,
                                                 position,
                                                 speed_of_sound,
                                                 sample_rate);

    const auto tail = raytracer::stochastic::postprocess(std::get<1>(input),
                                                         method,
                                                         position,
                                                         room_volume,
                                                         acoustic_impedance,
                                                         speed_of_sound,
                                                         sample_rate);

    return sum_vectors(head, tail);
}

}  // namesapce raytracer
