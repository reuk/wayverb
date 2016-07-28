#pragma once

#include "raytracer/cl_structs.h"
#include "raytracer/random_directions.h"
#include "raytracer/raytracer_program.h"
#include "raytracer/results.h"

#include "common/aligned/map.h"
#include "common/cl_include.h"
#include "common/hrtf_utils.h"
#include "common/scene_data.h"
#include "common/sinc.h"

#include <array>
#include <cmath>
#include <numeric>
#include <set>
#include <vector>
#include <random>

#include <experimental/optional>

namespace raytracer {

/// Get the number of necessary reflections for a given min amplitude.
int compute_optimum_reflection_number(float min_amp, float max_reflectivity);

class raytracer final {
public:
    raytracer(const cl::Context&, const cl::Device&);

    using PerStepCallback = std::function<void()>;

    std::experimental::optional<results> run(
            const CopyableSceneData& scene_data,
            const glm::vec3& source,
            const glm::vec3& receiver,
            size_t rays,
            size_t reflections,
            size_t image_source,
            std::atomic_bool& keep_going,
            const PerStepCallback& callback);

private:
    cl::Context context;
    cl::Device device;
};

}  // namespace raytracer
