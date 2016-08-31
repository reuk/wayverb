#pragma once

#include "raytracer/cl/structs.h"

#include "common/program_wrapper.h"
#include "common/spatial_division/scene_buffers.h"

namespace raytracer {

class program final {
public:
    program(const compute_context& cc, double speed_of_sound);

    auto get_kernel() const {
        return program_wrapper.get_kernel<cl::Buffer,  //  ray
                                          cl_float3,   //  receiver
                                          cl::Buffer,  //  voxel_index
                                          aabb,        //  global_aabb
                                          cl_uint,     //  side
                                          cl::Buffer,  //  triangles
                                          cl::Buffer,  //  vertices
                                          cl::Buffer,  //  surfaces
                                          cl::Buffer,  //  rng
                                          cl::Buffer   //  reflection
                                          >("reflections");
    }

    template <cl_program_info T>
    auto get_info() const {
        return program_wrapper.template get_info<T>();
    }

    cl::Device get_device() const { return program_wrapper.get_device(); }

private:
    program_wrapper program_wrapper;
};

}  // namespace raytracer
