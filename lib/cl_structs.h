#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

typedef enum : cl_int {
    id_none = 0,
    id_nx = 1 << 1,
    id_px = 1 << 2,
    id_ny = 1 << 3,
    id_py = 1 << 4,
    id_nz = 1 << 5,
    id_pz = 1 << 6,
    id_reentrant = 1 << 7,
} BoundaryType;

template <int P>
struct __attribute__((aligned(8))) NodeStruct {
    static constexpr int PORTS{P};
    cl_int ports[PORTS];
    cl_float3 position;
    bool inside;
    cl_int bt;
};
