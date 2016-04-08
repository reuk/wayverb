#pragma once

#include "cl_include.h"

typedef union {
    cl_float f;
    cl_uint i;
} __attribute__((aligned(8))) FloatUInt;

inline FloatUInt to_fui(cl_uint i) {
    FloatUInt ret;
    ret.i = i;
    return ret;
}

inline FloatUInt to_fui(cl_float f) {
    FloatUInt ret;
    ret.f = f;
    return ret;
}
