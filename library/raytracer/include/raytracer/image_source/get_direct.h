#pragma once

#include "raytracer/cl/structs.h"

#include "common/channels.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/unit_constructor.h"

namespace raytracer {
namespace image_source {

/// If there is line-of-sight between source and receiver, return the relative
/// time and intensity of the generated impulse.
template <typename Vertex, typename Surface>
auto get_direct(const glm::vec3& source,
                const glm::vec3& receiver,
                const voxelised_scene_data<Vertex, Surface>& scene_data) {
    constexpr auto channels = channels_v<Surface>;

    if (source == receiver) {
        return std::experimental::optional<impulse<channels>>{};
    }

    const auto source_to_receiver = receiver - source;
    const auto source_to_receiver_length = glm::length(source_to_receiver);
    const auto direction = glm::normalize(source_to_receiver);
    const geo::ray to_receiver{source, direction};

    const auto intersection = intersects(scene_data, to_receiver);

    if (!intersection ||
        (intersection && intersection->inter.t >= source_to_receiver_length)) {
        return std::experimental::make_optional(impulse<channels>{
                unit_constructor_v<
                        ::detail::cl_vector_constructor_t<float, channels>>,
                to_cl_float3(source),
                source_to_receiver_length});
    }

    return std::experimental::optional<impulse<channels>>{};
}

}  // namespace image_source
}  // namespace raytracer
