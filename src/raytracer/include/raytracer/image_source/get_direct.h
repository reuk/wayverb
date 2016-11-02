#pragma once

#include "raytracer/cl/structs.h"

#include "core/spatial_division/voxelised_scene_data.h"
#include "core/unit_constructor.h"

namespace wayverb {
namespace raytracer {
namespace image_source {

/// If there is line-of-sight between source and receiver, return the relative
/// time and intensity of the generated impulse.
template <typename Vertex, typename Surface>
auto get_direct(const glm::vec3& source,
                const glm::vec3& receiver,
                const core::voxelised_scene_data<Vertex, Surface>& scene_data) {
    constexpr auto channels = typename Surface::bands_t{};

    if (source == receiver) {
        return std::experimental::optional<impulse<channels>>{};
    }

    const auto source_to_receiver = receiver - source;
    const auto source_to_receiver_length = glm::length(source_to_receiver);
    const auto direction = glm::normalize(source_to_receiver);
    const core::geo::ray to_receiver{source, direction};

    const auto intersection = intersects(scene_data, to_receiver);

    if (!intersection ||
        (intersection && intersection->inter.t >= source_to_receiver_length)) {
        return std::experimental::make_optional(impulse<channels>{
                core::unit_constructor_v<
                        ::detail::cl_vector_constructor_t<float, channels>>,
                core::to_cl_float3(source),
                source_to_receiver_length});
    }

    return std::experimental::optional<impulse<channels>>{};
}

}  // namespace image_source
}  // namespace raytracer
}  // namespace wayverb
