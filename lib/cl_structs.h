#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

typedef struct {
    cl_int ports[4];
    cl_float3 position;
} _LinkedNode_unalign;

typedef _LinkedNode_unalign __attribute__((aligned(8))) LinkedTetrahedralNode;

typedef struct { cl_bool inside; } _UnlinkedNode_unalign;

typedef _UnlinkedNode_unalign
    __attribute__((aligned(8))) UnlinkedTetrahedralNode;

typedef struct {
    cl_ulong v0;
    cl_ulong v1;
    cl_ulong v2;
} _Triangle_unalign;

typedef _Triangle_unalign __attribute__((aligned(8))) Triangle;
