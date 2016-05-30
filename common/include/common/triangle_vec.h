#pragma once

#include "cl_include.h"

#include "glm/glm.hpp"

#include <array>

struct Triangle;

using TriangleVec3 = std::array<glm::vec3, 3>;

TriangleVec3 get_triangle_verts(const Triangle& t,
                                const std::vector<glm::vec3>& v);
TriangleVec3 get_triangle_verts(const Triangle& t,
                                const std::vector<cl_float3>& v);
