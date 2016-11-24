#pragma once

#include "raytracer/canonical.h"
#include "raytracer/image_source/postprocess.h"
#include "raytracer/stochastic/postprocess.h"

#include "core/sum_ranges.h"
#include "core/dsp_vector_ops.h"

namespace wayverb {
namespace raytracer {

template <typename Histogram, typename Method>
auto postprocess(const simulation_results<Histogram>& input,
                 const Method& method,
                 const glm::vec3& position,
                 double room_volume,
                 const core::environment& environment,
                 double output_sample_rate) {
    const auto head =
            raytracer::image_source::postprocess(begin(input.image_source),
                                                 end(input.image_source),
                                                 method,
                                                 position,
                                                 environment.speed_of_sound,
                                                 output_sample_rate);

    std::cout << "head max: " << core::max_mag(head) << '\n';
                                                 
    const auto tail = raytracer::stochastic::postprocess(input.stochastic,
                                                         method,
                                                         room_volume,
                                                         environment,
                                                         output_sample_rate);

    std::cout << "tail max: " << core::max_mag(tail) << '\n';

    return core::sum_vectors(head, tail);
}

}  // namesapce raytracer
}  // namespace wayverb
