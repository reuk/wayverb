#pragma once

#include "string_builder.h"

//#include <glog/logging.h>

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <iostream>

void print_device_info(const cl::Device& i);
cl::Context get_context();
cl::Device get_device(const cl::Context& context);

struct ComputeContext {
    ComputeContext();

    const cl::Context context;
    const cl::Device device;
    cl::CommandQueue queue;
};

template <typename T>
T get_program(const cl::Context& context, const cl::Device& device) {
    T program(context);
    try {
        program.build({device}, "-Werror");
    } catch (const cl::Error& e) {
        std::cerr << program.template getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)
                  << std::endl;
        std::cerr << e.what() << std::endl;
        throw;
    }
    return program;
}

template <typename T>
T get_program(const ComputeContext& context) {
    return get_program<T>(context.context, context.device);
}
