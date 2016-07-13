#pragma once

//#include <glog/logging.h>

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <iostream>

class ComputeContext final {
public:
    ComputeContext();

    const cl::Context context;
    const cl::Device device;
};
