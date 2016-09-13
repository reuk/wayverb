#pragma once

#include "raytracer/results.h"

#include "common/cl/common.h"
#include "common/cl/geometry.h"

#include <experimental/optional>

class voxelised_scene_data;

namespace raytracer {

/// If there is line-of-sight between source and receiver, return the relative
/// time and intensity of the generated impulse.
std::experimental::optional<impulse> get_direct_impulse(
        const glm::vec3& source,
        const glm::vec3& receiver,
        const voxelised_scene_data& scene_data,
        double speed_of_sound);

/// arguments
///     the step number
using per_step_callback = std::function<void(size_t)>;

using reflection_processor =
        std::function<void(const aligned::vector<reflection>&)>;

std::experimental::optional<results> run(
        const compute_context& cc,
        const voxelised_scene_data& scene_data,
        double speed_of_sound,
        double acoustic_impedance,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const aligned::vector<glm::vec3>& directions,
        size_t reflections,
        size_t image_source,
        const std::atomic_bool& keep_going,
        const per_step_callback& callback);

}  // namespace raytracer
