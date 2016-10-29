#include "raytracer/raytracer.h"

namespace raytracer {

std::tuple<reflection_processor::make_image_source,
           reflection_processor::make_directional_histogram,
           reflection_processor::make_visual>
make_canonical_callbacks(size_t max_image_source_order, size_t visual_items) {
    return std::make_tuple(
            raytracer::reflection_processor::make_image_source{
                    max_image_source_order, true},
            raytracer::reflection_processor::make_directional_histogram{
                    0.1f, 1000.0f, max_image_source_order + 1},
            raytracer::reflection_processor::make_visual{visual_items});
}

}  // namespace raytracer
