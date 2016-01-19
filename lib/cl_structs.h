#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

enum NodeType : cl_int {
    id_inside,
    id_boundary,
    id_outside,
};

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
