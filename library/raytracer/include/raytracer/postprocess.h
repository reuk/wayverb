#pragma once

#include "raytracer/image_source/postprocess.h"
#include "raytracer/raytracer.h"
#include "raytracer/stochastic/postprocess.h"

#include "common/sum_ranges.h"

namespace raytracer {

template <typename Histogram, typename Method>
auto postprocess(const aural_results<Histogram>& input,
                 const Method& method,
                 const glm::vec3& position,
                 double room_volume,
                 double acoustic_impedance,
                 double speed_of_sound,
                 double output_sample_rate) {
    const auto head =
            raytracer::image_source::postprocess(begin(input.image_source),
                                                 end(input.image_source),
                                                 method,
                                                 position,
                                                 speed_of_sound,
                                                 output_sample_rate);

    const auto tail = raytracer::stochastic::postprocess(input.stochastic,
                                                         method,
                                                         room_volume,
                                                         acoustic_impedance,
                                                         speed_of_sound,
                                                         output_sample_rate);

    return sum_vectors(head, tail);
}

}  // namesapce raytracer
