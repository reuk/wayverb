#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

typedef struct {
    cl_int ports[4];
    cl_float3 position;
    cl_bool inside;
} __attribute__((aligned(8))) Node;
