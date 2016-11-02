#pragma once

#include "raytracer/canonical.h"
#include "raytracer/image_source/postprocess.h"
#include "raytracer/stochastic/postprocess.h"

#include "core/sum_ranges.h"

namespace wayverb {
namespace raytracer {

template <typename Histogram, typename Method>
auto postprocess(const simulation_results<Histogram>& input,
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

    return core::sum_vectors(head, tail);
}

}  // namesapce raytracer
}  // namespace wayverb
