#include "core/geo/triangle_vec.h"
#include "core/cl/triangle.h"
#include "core/conversions.h"

namespace wayverb {
namespace core {
namespace geo {

triangle_vec3 get_triangle_vec3(const triangle& t, const glm::vec3* v) {
    return triangle_vec3{{{v[t.v0], v[t.v1], v[t.v2]}}};
}

triangle_vec3 get_triangle_vec3(const triangle& t, const cl_float3* v) {
    return triangle_vec3{
            {{to_vec3{}(v[t.v0]), to_vec3{}(v[t.v1]), to_vec3{}(v[t.v2])}}};
}

////////////////////////////////////////////////////////////////////////////////

float area(const triangle_vec3& triangle) {
    const auto v0 = triangle.s[1] - triangle.s[0];
    const auto v1 = triangle.s[2] - triangle.s[0];

    const auto a = std::pow(v0.y * v1.z - v0.z * v1.y, 2);
    const auto b = std::pow(v0.z * v1.x - v0.x * v1.z, 2);
    const auto c = std::pow(v0.x * v1.y - v0.y * v1.x, 2);
    return std::sqrt(a + b + c) / 2;
}

}  // namespace geo
}  // namespace core
}  // namespace wayverb
