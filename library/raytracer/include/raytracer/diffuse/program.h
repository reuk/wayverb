#pragma once

#include "raytracer/cl/structs.h"

#include "common/program_wrapper.h"
#include "common/spatial_division/scene_buffers.h"

namespace raytracer {
namespace diffuse {

class program final {
public:
    program(const compute_context& cc);

    auto get_kernel() const {
        return program_wrapper_.get_kernel<cl::Buffer,  // reflections
                                           cl_float3,   // receiver
                                           cl_float,    // receiver radius
                                           cl::Buffer,  // triangles
                                           cl::Buffer,  // vertices
                                           cl::Buffer,  // surfaces
                                           cl::Buffer,  // diffuse path info
                                           cl::Buffer,  // diffuse output
                                           cl::Buffer   // intersected output
                                           >("diffuse");
    }

    auto get_init_diffuse_path_info_kernel() const {
        return program_wrapper_.get_kernel<cl::Buffer,   // buffer
                                           volume_type,  // initial volume
                                           cl_float3     // initial position
                                           >("init_diffuse_path_info");
    }

private:
    program_wrapper program_wrapper_;
};

}  // namespace diffuse
}  // namespace raytracer
