#include "raytracer/canonical.h"

namespace raytracer {

std::tuple<reflection_processor::make_image_source,
           reflection_processor::make_directional_histogram,
           reflection_processor::make_visual>
make_canonical_callbacks(size_t max_image_source_order, size_t visual_items) {
    /// Flipping phase gives more natural-looking waveforms, but gives incorrect
    /// frequency response.
    constexpr auto flip_phase = false;

    /// Receiver is human-head-sized (value is in metres).
    constexpr auto receiver_radius = 0.1f;

    /// Higher == more detailed reverb tail, higher memory usage.
    constexpr auto histogram_sample_rate = 1000.0f;

    return std::make_tuple(
            raytracer::reflection_processor::make_image_source{
                    max_image_source_order, flip_phase},
            raytracer::reflection_processor::make_directional_histogram{
                    receiver_radius,
                    histogram_sample_rate,
                    max_image_source_order + 1},
            raytracer::reflection_processor::make_visual{visual_items});
}

}  // namespace raytracer
