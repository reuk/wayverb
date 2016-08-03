#pragma once

#include "cl_structs.h"

#include "common/program_wrapper.h"

namespace raytracer {

class program final {
public:
    program(const cl::Context& context, const cl::Device& device);

    auto get_reflections_kernel() const {
        return program_wrapper.get_kernel<cl::Buffer,  //  ray
                                          cl_float3,   //  receiver
                                          cl::Buffer,  //  voxel_index
                                          AABB,        //  global_aabb
                                          cl_ulong,    //  side
                                          cl::Buffer,  //  triangles
                                          cl::Buffer,  //  vertices
                                          cl::Buffer,  //  surfaces
                                          cl::Buffer,  //  rng
                                          cl::Buffer   //  reflection
                                          >("reflections");
    }

    auto get_diffuse_kernel() const {
        return program_wrapper.get_kernel<cl::Buffer,  // reflections
                                          cl_float3,   // receiver
                                          VolumeType,  // air_coefficient
                                          cl::Buffer,  // triangles
                                          cl::Buffer,  // vertices
                                          cl::Buffer,  // surfaces
                                          cl::Buffer,  // diffuse_path_info
                                          cl::Buffer   // diffuse_output
                                          >("diffuse");
    }

    template <cl_program_info T>
    auto get_info() const {
        return program_wrapper.template get_info<T>();
    }

    cl::Device get_device() const { return program_wrapper.get_device(); }

private:
    static const std::string source;

    program_wrapper program_wrapper;
};

}  // namespace raytracer
