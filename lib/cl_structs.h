#pragma once

#define __CL_ENABLE_EXCEPTIONS
#include "cl.hpp"

typedef enum : cl_int {
    id_inside = 1,
    id_boundary,
    id_outside,
} NodeType;

typedef enum : cl_int {
    id_none = 0,
    id_left = 1 << 1,
    id_right = 1 << 2,
    id_front = 1 << 3,
    id_back = 1 << 4,
    id_top = 1 << 5,
    id_bottom = 1 << 6,
    id_left_front = id_left | id_front,
    id_left_back = id_left | id_back,
    id_left_top = id_left | id_top,
    id_left_bottom = id_left | id_bottom,
    id_right_front = id_right | id_front,
    id_right_back = id_right | id_back,
    id_right_top = id_right | id_top,
    id_right_bottom = id_right | id_bottom,
    id_front_top = id_front | id_top,
    id_front_bottom = id_front | id_bottom,
    id_back_top = id_back | id_top,
    id_back_bottom = id_back | id_bottom,
    id_left_front_top = id_left | id_front | id_top,
    id_left_front_bottom = id_left | id_front | id_bottom,
    id_left_back_top = id_left | id_back | id_top,
    id_left_back_bottom = id_left | id_back | id_bottom,
    id_right_front_top = id_right | id_front | id_top,
    id_right_front_bottom = id_right | id_front | id_bottom,
    id_right_back_top = id_right | id_back | id_top,
    id_right_back_bottom = id_right | id_back | id_bottom,
} BoundaryType;

template <int P>
struct __attribute__((aligned(8))) NodeStruct {
    static constexpr int PORTS{P};
    cl_int ports[PORTS];
    cl_float3 position;
    NodeType inside;
    BoundaryType bt;
};

using KNode = NodeStruct<4>;
using RectNode = NodeStruct<6>;
