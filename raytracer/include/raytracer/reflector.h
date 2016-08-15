#pragma once

#include "raytracer/program.h"

#include "common/aligned/vector.h"
#include "common/cl/geometry.h"
#include "common/cl_include.h"

#include "glm/glm.hpp"

class scene_buffers;

namespace raytracer {

aligned::vector<glm::vec3> get_random_directions(size_t num);
aligned::vector<ray> get_random_rays(size_t num, const glm::vec3& source);

class reflector final {
public:
    reflector(const cl::Context&,
              const cl::Device&,
              const glm::vec3& source,
              const glm::vec3& receiver,
              const aligned::vector<glm::vec3>& directions);

    aligned::vector<reflection> run_step(const scene_buffers& buffers);

private:
    using kernel_t = decltype(
            std::declval<program>().get_reflections_kernel());

    cl::Context context;
    cl::Device device;
    cl::CommandQueue queue;
    kernel_t kernel;
    cl_float3 receiver;
    size_t rays;

    cl::Buffer ray_buffer;
    cl::Buffer reflection_buffer;

    cl::Buffer rng_buffer;
};

}  // namespace raytracer
