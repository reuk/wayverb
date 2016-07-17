#pragma once

#include "cl_structs.h"

#include "common/custom_program_base.h"

class raytracer_program final {
public:
    raytracer_program(const cl::Context& context, const cl::Device& device);

    auto get_raytrace_kernel() const {
        return custom_program_base.get_kernel<cl::Buffer,  //  ray_info
                                              cl::Buffer,  //  voxel_index
                                              AABB,        //  global_aabb
                                              cl_int,      //  side
                                              cl::Buffer,  //  triangles
                                              cl_ulong,    //  numtriangles
                                              cl::Buffer,  //  vertices
                                              cl::Buffer,  //  surfaces
                                              cl_float3,   //  source
                                              cl_float3,   //  mic
                                              VolumeType,  //  air_coefficient
                                              cl_ulong,    //  iteration
                                              cl_ulong,    //  num_image_source
                                              cl::Buffer,  //  impulses
                                              cl::Buffer,  //  image_source
                                              cl::Buffer,  //  prev_primitives
                                              cl::Buffer  //  image_source_index
                                              >("raytrace");
    }

    template <cl_program_info T>
    auto get_info() const {
        return custom_program_base.template get_info<T>();
    }

    cl::Device get_device() const {
        return custom_program_base.get_device();
    }

private:
    static const std::string source;

    custom_program_base custom_program_base;
};
