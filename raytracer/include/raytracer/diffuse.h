#pragma once

#include "raytracer/cl/structs.h"
#include "raytracer/iterative_builder.h"
#include "raytracer/program.h"

#include "common/aligned/vector.h"
#include "common/cl_common.h"
#include "common/conversions.h"
#include "common/spatial_division/scene_buffers.h"

namespace raytracer {

class diffuse_finder final {
public:
    diffuse_finder(const cl::Context&,
                   const cl::Device&,
                   const glm::vec3& source,
                   const glm::vec3& receiver,
                   const volume_type& air_coefficient,
                   double speed_of_sound,
                   size_t rays,
                   size_t depth);

    void push(const aligned::vector<reflection>& reflections,
              const scene_buffers& scene_buffers);
    const aligned::vector<aligned::vector<impulse>>& get_results() const;
    aligned::vector<aligned::vector<impulse>>& get_results();

private:
    using kernel_t = decltype(std::declval<program>().get_diffuse_kernel());

    cl::Context context;
    cl::Device device;
    cl::CommandQueue queue;
    kernel_t kernel;
    cl_float3 receiver;
    volume_type air_coefficient;
    size_t rays;

    cl::Buffer reflections_buffer;
    cl::Buffer diffuse_path_buffer;
    cl::Buffer impulse_buffer;

    iterative_builder<impulse> impulse_builder;
};

}  // namespace raytracer
