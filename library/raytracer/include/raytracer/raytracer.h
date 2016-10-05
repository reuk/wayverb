#pragma once

#include "raytracer/image_source/run.h"
#include "raytracer/results.h"

#include "common/cl/common.h"
#include "common/cl/geometry.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include <experimental/optional>

namespace raytracer {

/// If there is line-of-sight between source and receiver, return the relative
/// time and intensity of the generated impulse.
template <typename Vertex, typename Surface>
auto get_direct(const glm::vec3& source,
                const glm::vec3& receiver,
                const voxelised_scene_data<Vertex, Surface>& scene_data) {
    constexpr auto channels{
            ::detail::components_v<specular_absorption_t<Surface>>};

    if (source == receiver) {
        return std::experimental::optional<impulse<channels>>{};
    }

    const auto source_to_receiver{receiver - source};
    const auto source_to_receiver_length{glm::length(source_to_receiver)};
    const auto direction{glm::normalize(source_to_receiver)};
    const geo::ray to_receiver{source, direction};

    const auto intersection{intersects(scene_data, to_receiver)};

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

/// arguments
///     the step number
using per_step_callback = std::function<void(size_t)>;

using reflection_processor =
        std::function<void(const aligned::vector<reflection>&)>;

std::experimental::optional<results<impulse<8>>> run(
        const compute_context& cc,
        const voxelised_scene_data<cl_float3, surface>& scene_data,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const aligned::vector<glm::vec3>& directions,
        size_t reflections,
        size_t image_source,
        const std::atomic_bool& keep_going,
        const per_step_callback& callback);

}  // namespace raytracer
