#include "common/geo/triangle_vec.h"
#include "common/conversions.h"
#include "common/triangle.h"

namespace geo {

triangle_vec3 get_triangle_vec3(const triangle& t,
                                const aligned::vector<glm::vec3>& v) {
    return triangle_vec3{{v[t.v0], v[t.v1], v[t.v2]}};
}

triangle_vec3 get_triangle_vec3(const triangle& t,
                                const aligned::vector<cl_float3>& v) {
    return triangle_vec3{
            {to_vec3(v[t.v0]), to_vec3(v[t.v1]), to_vec3(v[t.v2])}};
}

triangle_vec2 get_triangle_vec2(const triangle& t,
                                const aligned::vector<glm::vec3>& v) {
    return triangle_vec2{{v[t.v0], v[t.v1], v[t.v2]}};
}

triangle_vec2 get_triangle_vec2(const triangle& t,
                                const aligned::vector<cl_float3>& v) {
    return triangle_vec2{
            {to_vec3(v[t.v0]), to_vec3(v[t.v1]), to_vec3(v[t.v2])}};
}

}  // namespace geo
