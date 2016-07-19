#pragma once

//#include <glog/logging.h>

#include "stl_wrappers.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <iostream>

class ComputeContext final {
public:
    ComputeContext();

    const cl::Context context;
    const cl::Device device;
};

template <typename T>
bool cl_equal(const T& a, const T& b) {
    return proc::equal(a.s, std::begin(b.s));
}

inline bool operator==(const cl_float3& a, const cl_float3& b) {
        return cl_equal(a, b);
}

inline bool operator!=(const cl_float3& a, const cl_float3& b) {
        return !(a == b);
}
