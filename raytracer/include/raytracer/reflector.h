#pragma once

#include "raytracer/program.h"

#include "common/aligned/vector.h"
#include "common/cl_include.h"

namespace raytracer {

class scene_buffers;

class reflector final {
public:
    reflector(const cl::Context&,
              const cl::Device&,
              const glm::vec3& source,
              const glm::vec3& receiver,
              size_t rays);

    aligned::vector<reflection> run_step(scene_buffers& buffers);

private:
    using kernel_t = decltype(
            std::declval<program>().get_reflections_kernel());

    cl::Context context;
    cl::Device device;
    kernel_t kernel;
    cl_float3 receiver;
    size_t rays;

    cl::Buffer ray_buffer;
    cl::Buffer reflection_buffer;

    cl::Buffer rng_buffer;
};

}  // namespace raytracer
