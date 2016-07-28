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

#define DEFINE_CL_EQUALITY_OP(type)                        \
    inline bool operator==(const type& a, const type& b) { \
        return cl_equal(a, b);                             \
    }                                                      \
    inline bool operator!=(const type& a, const type& b) { return !(a == b); }

#define DEFINE_CL_VECTOR_EQUALITY_OP(type_prefix_) \
    DEFINE_CL_EQUALITY_OP(type_prefix_##2)         \
    DEFINE_CL_EQUALITY_OP(type_prefix_##4)         \
    DEFINE_CL_EQUALITY_OP(type_prefix_##8)         \
    DEFINE_CL_EQUALITY_OP(type_prefix_##16)

//  charn, ucharn, shortn, ushortn, intn, uintn, longn, ulongn, floatn, doublen

DEFINE_CL_VECTOR_EQUALITY_OP(cl_char)
DEFINE_CL_VECTOR_EQUALITY_OP(cl_uchar)
DEFINE_CL_VECTOR_EQUALITY_OP(cl_short)
DEFINE_CL_VECTOR_EQUALITY_OP(cl_ushort)
DEFINE_CL_VECTOR_EQUALITY_OP(cl_int)
DEFINE_CL_VECTOR_EQUALITY_OP(cl_uint)
DEFINE_CL_VECTOR_EQUALITY_OP(cl_long)
DEFINE_CL_VECTOR_EQUALITY_OP(cl_ulong)
DEFINE_CL_VECTOR_EQUALITY_OP(cl_float)
DEFINE_CL_VECTOR_EQUALITY_OP(cl_double)

template <typename T>
cl::Buffer load_to_buffer(const cl::Context& context, T t, bool read_only) {
    return cl::Buffer(context, std::begin(t), std::end(t), read_only);
}
