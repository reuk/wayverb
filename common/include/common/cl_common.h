#pragma once

//#include <glog/logging.h>

#include "stl_wrappers.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

#include <iostream>

class compute_context final {
public:
    compute_context();

    cl::Context get_context() const;
    cl::Device get_device() const;

private:
    cl::Context context;
    cl::Device device;
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

template <typename T>
cl::Buffer load_to_buffer(const cl::Context& context, T t, bool read_only) {
    return cl::Buffer(context, std::begin(t), std::end(t), read_only);
}
