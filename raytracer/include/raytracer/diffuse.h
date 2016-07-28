#pragma once

#include "raytracer/cl_structs.h"
#include "raytracer/iterative_builder.h"
#include "raytracer/scene_buffers.h"

#include "common/aligned/vector.h"
#include "common/cl_common.h"
#include "common/conversions.h"

namespace raytracer {

class diffuse_finder final {
public:
    diffuse_finder(const cl::Context&,
                   const cl::Device&,
                   const glm::vec3& receiver,
                   size_t rays,
                   size_t depth);

    void push(const aligned::vector<Reflection>& reflections,
              scene_buffers& scene_buffers);
    const aligned::vector<aligned::vector<Impulse>>& get_results() const;
    aligned::vector<aligned::vector<Impulse>>& get_results();

private:
    cl::Context context;
    cl::Device device;
    cl_float3 receiver;
//    size_t rays;

    cl::Buffer reflection_buffer;
    cl::Buffer diffuse_path_buffer;
    cl::Buffer impulse_buffer;

    iterative_builder<Impulse> impulse_builder;
};

}  // namespace raytracer
