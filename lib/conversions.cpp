#include "conversions.h"

#include "vec.h"

#define VEC_CONVERT_FUNCTIONS_DEFINITION(t)     \
    cl_##t##3 convert(const Vec3<t> & v) {      \
        return {{v.x, v.y, v.z}};               \
    }                                           \
    Vec3<t> convert(const cl_##t##3 & v) {      \
        return Vec3<t>(v.x, v.y, v.z);          \
    }

VEC_CONVERT_FUNCTIONS_DEFINITION(float)
VEC_CONVERT_FUNCTIONS_DEFINITION(int)
