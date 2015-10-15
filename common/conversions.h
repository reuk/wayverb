#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

template <typename T>
struct Vec3;

#define VEC_CONVERT_FUNCTIONS_DECLARATION(t) \
    cl_##t##3 convert(const Vec3<t> & v);    \
    Vec3<t> convert(const cl_##t##3 & v);

VEC_CONVERT_FUNCTIONS_DECLARATION(float)
VEC_CONVERT_FUNCTIONS_DECLARATION(int)
