#pragma once

#include "core/cl/include.h"

#include "glm/glm.hpp"

#include <array>

namespace wayverb {
namespace core {

struct triangle;

namespace geo {

struct triangle_vec3 final {
    std::array<glm::vec3, 3> s;
};

triangle_vec3 get_triangle_vec3(const triangle& t, const glm::vec3* v);
triangle_vec3 get_triangle_vec3(const triangle& t, const cl_float3* v);

float area(const triangle_vec3& triangle);

}  // namespace geo
}  // namespace core
}  // namespace wayverb
