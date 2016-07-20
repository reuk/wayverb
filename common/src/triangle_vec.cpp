#include "common/conversions.h"
#include "common/triangle.h"
#include "common/triangle_vec.h"

TriangleVec3 get_triangle_verts(const Triangle& t,
                                const aligned::vector<glm::vec3>& v) {
    return TriangleVec3{{v[t.v0], v[t.v1], v[t.v2]}};
}

TriangleVec3 get_triangle_verts(const Triangle& t,
                                const aligned::vector<cl_float3>& v) {
    return TriangleVec3{
            {to_vec3f(v[t.v0]), to_vec3f(v[t.v1]), to_vec3f(v[t.v2])}};
}
