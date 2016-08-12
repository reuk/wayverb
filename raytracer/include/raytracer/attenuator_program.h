#pragma once

#include "cl/structs.h"

#include "common/program_wrapper.h"

namespace raytracer {

class attenuator_program final {
public:
    attenuator_program(const cl::Context& context, const cl::Device& device);

    auto get_microphone_kernel() const {
        return program_wrapper
                .get_kernel<cl_float3, cl::Buffer, cl::Buffer, microphone>(
                        "microphone");
    }

    auto get_hrtf_kernel() const {
        return program_wrapper.get_kernel<cl_float3,
                                          cl::Buffer,
                                          cl::Buffer,
                                          cl::Buffer,
                                          cl_float3,
                                          cl_float3,
                                          cl_ulong>("hrtf");
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
