#pragma once

#include "cl_structs.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

class RayverbProgram : public cl::Program {
public:
    RayverbProgram(const cl::Context& context, bool build_immediate = false);

    auto get_raytrace_kernel() const {
        return cl::make_kernel<cl::Buffer,
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
                               VolumeType>(*this, "raytrace");
    }

    auto get_improved_raytrace_kernel() const {
        return cl::make_kernel<cl::Buffer,
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
                               cl_ulong>(*this, "raytrace_improved");
    }

    auto get_attenuate_kernel() const {
        return cl::make_kernel<cl_float3, cl::Buffer, cl::Buffer, Speaker>(
            *this, "attenuate");
    }

    auto get_hrtf_kernel() const {
        return cl::make_kernel<cl_float3,
                               cl::Buffer,
                               cl::Buffer,
                               cl::Buffer,
                               cl_float3,
                               cl_float3,
                               cl_ulong>(*this, "hrtf");
    }

private:
    static const std::string source;
};
