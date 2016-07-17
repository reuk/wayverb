#pragma once

#include "cl_structs.h"

#include "common/custom_program_base.h"

class attenuator_program final {
public:
    attenuator_program(const cl::Context& context, const cl::Device& device);

    auto get_microphone_kernel() const {
        return custom_program_base
                .get_kernel<cl_float3, cl::Buffer, cl::Buffer, Speaker>(
                        "microphone");
    }

    auto get_hrtf_kernel() const {
        return custom_program_base.get_kernel<cl_float3,
                                              cl::Buffer,
                                              cl::Buffer,
                                              cl::Buffer,
                                              cl_float3,
                                              cl_float3,
                                              cl_ulong>("hrtf");
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
