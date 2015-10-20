#pragma once

#include "logger.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

using namespace std;

void print_device_info(const cl::Device & i);
cl::Context get_context();
cl::Device get_device(const cl::Context & context);

template <typename T>
T get_program(const cl::Context & context, const cl::Device & device) {
    T program(context);
    try {
        program.build({device});
    } catch (const cl::Error & e) {
        Logger::log(
            program.template getBuildInfo<CL_PROGRAM_BUILD_LOG>(device));
        throw;
    }
    return program;
}
