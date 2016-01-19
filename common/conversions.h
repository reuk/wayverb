#pragma once

#include "vec.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

template <typename T>
inline cl_float3 to_cl_float3(const T& t) {
    return cl_float3{{t.x, t.y, t.z, 0}};
}

template <typename T>
inline Vec3f to_vec3f(const T& t) {
    return Vec3f{t.x, t.y, t.z};
}

template<>
inline Vec3f to_vec3f(const cl_float3& t) {
    return Vec3f{t.s[0], t.s[1], t.s[2]};
}
