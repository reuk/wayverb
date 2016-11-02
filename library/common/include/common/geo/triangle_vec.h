#pragma once

#include "common/cl/include.h"

#include "utilities/aligned/vector.h"

#include "glm/glm.hpp"

#include <array>

struct triangle;

namespace geo {

using triangle_vec3 = std::array<glm::vec3, 3>;

triangle_vec3 get_triangle_vec3(const triangle& t,
                                const util::aligned::vector<glm::vec3>& v);
triangle_vec3 get_triangle_vec3(const triangle& t,
                                const util::aligned::vector<cl_float3>& v);

using triangle_vec2 = std::array<glm::vec2, 3>;

triangle_vec2 get_triangle_vec2(const triangle& t,
                                const util::aligned::vector<glm::vec3>& v);
triangle_vec2 get_triangle_vec2(const triangle& t,
                                const util::aligned::vector<cl_float3>& v);

////////////////////////////////////////////////////////////////////////////////

float area(const triangle_vec3& triangle);

}  // namespace geo
