#pragma once

#include "vec.h"

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

template<typename T>
cl_float3 to_cl_float3(const T& t) {
    return cl_float3{{t.x, t.y, t.z, 0}};
}

template<typename T>
Vec3f to_vec3f(const T& t) {
    return Vec3f{t.x, t.y, t.z};
}
