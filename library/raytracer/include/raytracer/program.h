#pragma once

#include "raytracer/cl/structs.h"

#include "common/program_wrapper.h"
#include "common/spatial_division/scene_buffers.h"

namespace raytracer {

class program final {
public:
    program(const compute_context& cc);

    auto get_kernel() const {
        return program_wrapper_.get_kernel<cl::Buffer,  //  ray
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

    auto get_init_reflections_kernel() const {
        return program_wrapper_.get_kernel<cl::Buffer>("init_reflections");
    }

    template <cl_program_info T>
    auto get_info() const {
        return program_wrapper_.template get_info<T>();
    }

    cl::Device get_device() const { return program_wrapper_.get_device(); }

private:
    program_wrapper program_wrapper_;
};

}  // namespace raytracer
