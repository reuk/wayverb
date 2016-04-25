#include "common/triangle_vec.h"
#include "common/conversions.h"
#include "common/triangle.h"

#include "common/vec.h"

TriangleVec3f get_triangle_verts(const Triangle& t,
                                 const std::vector<Vec3f>& v) {
    return TriangleVec3f{{v[t.v0], v[t.v1], v[t.v2]}};
}

TriangleVec3f get_triangle_verts(const Triangle& t,
                                 const std::vector<cl_float3>& v) {
    return TriangleVec3f{
        {to_vec3f(v[t.v0]), to_vec3f(v[t.v1]), to_vec3f(v[t.v2])}};
}
