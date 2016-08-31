#pragma once

#include "raytracer/cl/structs.h"

#include "common/program_wrapper.h"
#include "common/spatial_division/scene_buffers.h"

namespace raytracer {
namespace diffuse {

class program final {
public:
    program(const compute_context& cc, double speed_of_sound);

    auto get_kernel() const {
        return program_wrapper_.get_kernel<cl::Buffer,  // reflections
                                           cl_float3,   // receiver
                                           cl::Buffer,  // triangles
                                           cl::Buffer,  // vertices
                                           cl::Buffer,  // surfaces
                                           cl::Buffer,  // diffuse_path_info
                                           cl::Buffer   // diffuse_output
                                           >("diffuse");
    }

private:
    program_wrapper program_wrapper_;
};

}  // namespace diffuse
}  // namespace raytracer
