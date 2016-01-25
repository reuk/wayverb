#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

typedef enum : cl_int {
    id_inside = 1,
    id_boundary,
    id_outside,
} NodeType;

typedef struct __KNode {
    static constexpr int PORTS{4};
    cl_int ports[PORTS];
    cl_float3 position;
    NodeType inside;
} __attribute__((aligned(8))) KNode;

typedef struct __RectNode {
    static constexpr int PORTS{6};
    cl_int ports[PORTS];
    cl_float3 position;
    NodeType inside;
} __attribute__((aligned(8))) RectNode;
