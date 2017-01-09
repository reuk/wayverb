#include "raytracer/canonical.h"

namespace wayverb {
namespace raytracer {

std::tuple<reflection_processor::make_image_source,
           reflection_processor::make_directional_histogram,
           reflection_processor::make_visual>
make_canonical_callbacks(const simulation_parameters& params,
                         size_t visual_items) {
    return std::make_tuple(
            raytracer::reflection_processor::make_image_source(
                    params.maximum_image_source_order),
            raytracer::reflection_processor::make_directional_histogram(
                    params.rays,
                    params.maximum_image_source_order + 1,
                    params.receiver_radius,
                    params.histogram_sample_rate),
            raytracer::reflection_processor::make_visual{visual_items});
}

}  // namespace raytracer
}  // namespace wayverb
