#pragma once

#include "cl_structs.h"

#include "common/custom_program_base.h"

class RaytracerProgram : public custom_program_base {
public:
    explicit RaytracerProgram(const cl::Context& context,
                              const cl::Device& device);

    auto get_raytrace_kernel() const {
        return get_kernel<cl::Buffer,
                          cl_float3,
                          cl::Buffer,
                          cl_ulong,
                          cl::Buffer,
                          cl_float3,
                          cl::Buffer,
                          cl::Buffer,
                          cl::Buffer,
                          cl::Buffer,
                          cl_ulong,
                          VolumeType>("raytrace");
    }

    auto get_improved_raytrace_kernel() const {
        return get_kernel<cl::Buffer,
                          cl::Buffer,
                          AABB,
                          cl_int,
                          cl::Buffer,
                          cl_ulong,
                          cl::Buffer,
                          cl::Buffer,
                          cl_float3,
                          cl_float3,
                          cl::Buffer,
                          cl::Buffer,
                          cl::Buffer,
                          VolumeType,
                          cl_ulong>("raytrace_improved");
    }

    auto get_attenuate_kernel() const {
        return get_kernel<cl_float3, cl::Buffer, cl::Buffer, Speaker>(
                "attenuate");
    }

    auto get_hrtf_kernel() const {
        return get_kernel<cl_float3,
                          cl::Buffer,
                          cl::Buffer,
                          cl::Buffer,
                          cl_float3,
                          cl_float3,
                          cl_ulong>("hrtf");
    }

private:
    static const std::string source;
};
