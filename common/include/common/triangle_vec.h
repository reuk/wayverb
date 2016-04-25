#pragma once

#include "vec_forward.h"

#include "cl_include.h"

#include <array>

struct Triangle;

using TriangleVec3f = std::array<Vec3f, 3>;

TriangleVec3f get_triangle_verts(const Triangle& t,
                                 const std::vector<Vec3f>& v);
TriangleVec3f get_triangle_verts(const Triangle& t,
                                 const std::vector<cl_float3>& v);
