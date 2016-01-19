#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

typedef enum : cl_int {
    id_inside = 1,
    id_boundary,
    id_outside,
} NodeType;

typedef struct {
    cl_int ports[4];
    cl_float3 position;
    NodeType inside;
} __attribute__((aligned(8))) KNode;

typedef struct {
    cl_int ports[4];
    cl_float i_var[4];
    cl_float o_var[4];
    cl_float3 position;
} __attribute__((aligned(8))) WNode;
