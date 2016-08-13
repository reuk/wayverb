#pragma once

#include "raytracer/results.h"
#include "common/cl/geometry.h"

#include <experimental/optional>

class voxelised_scene_data;

namespace raytracer {

/// Get the number of necessary reflections for a given min amplitude.
int compute_optimum_reflection_number(float min_amp, float max_reflectivity);

/// arguments
///     the step number
using per_step_callback = std::function<void(size_t)>;

std::experimental::optional<results> run(
        const cl::Context&,
        const cl::Device&,
        const voxelised_scene_data& scene_data,
        const glm::vec3& source,
        const glm::vec3& receiver,
        const aligned::vector<glm::vec3>& directions,
        size_t reflections,
        size_t image_source,
        std::atomic_bool& keep_going,
        const per_step_callback& callback);

}  // namespace raytracer
