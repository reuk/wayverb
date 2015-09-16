#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

typedef struct {
    cl_int ports[4];
    cl_float3 position;
    cl_bool inside;
} _Node_unalign;

typedef _Node_unalign __attribute__((aligned(8))) TetrahedralNode;

typedef struct {
    cl_ulong v0;
    cl_ulong v1;
    cl_ulong v2;
} _Triangle_unalign;

typedef _Triangle_unalign __attribute__((aligned(8))) Triangle;
