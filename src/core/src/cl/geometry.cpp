#include "core/cl/geometry.h"

namespace cl_sources {
const char* geometry{R"(
bool almost_equal(float x, float y, size_t ulp);
bool almost_equal(float x, float y, size_t ulp) {
    const float abs_diff = fabs(x - y);
    return abs_diff < FLT_EPSILON * fabs(x + y) * ulp || abs_diff < FLT_MIN;
}

#define ULP (10)

bool is_degenerate(triangle_inter i);
bool is_degenerate(triangle_inter i) {
    return almost_equal(i.u, 0, ULP) || almost_equal(i.v, 0, ULP) || almost_equal(i.u + i.v, 1, ULP);
}

triangle_inter triangle_vert_intersection(triangle_verts verts, ray r);
triangle_inter triangle_vert_intersection(triangle_verts verts, ray r) {
    const float3 e0 = verts.v1 - verts.v0;
    const float3 e1 = verts.v2 - verts.v0;

    const float3 pvec = cross(r.direction, e1);
    const float det = dot(e0, pvec);

    if (almost_equal(det, 0, ULP)) {
        return (triangle_inter){};
    }

    const float invdet = 1.0f / det;
    const float3 tvec = r.position - verts.v0;
    const float u = invdet * dot(tvec, pvec);

    if (u < 0.0f || 1.0f < u) {
        return (triangle_inter){};
    }

    const float3 qvec = cross(tvec, e0);
    const float v = invdet * dot(r.direction, qvec);

    if (v < 0.0f || 1.0f < v + u) {
        return (triangle_inter){};
    }

    const float t = invdet * dot(e1, qvec);

    if (t < 0 || almost_equal(t, 0, ULP)) {
        return (triangle_inter) {};
    }

    return (triangle_inter){t, u, v};
}

triangle_inter triangle_intersection(triangle triangle,
                            const global float3 * vertices,
                            ray r);
triangle_inter triangle_intersection(triangle triangle,
                            const global float3 * vertices,
                            ray r) {
    const triangle_verts v = {
        vertices[triangle.v0], vertices[triangle.v1], vertices[triangle.v2]};
    return triangle_vert_intersection(v, r);
}

float3 triangle_verts_normal(triangle_verts t);
float3 triangle_verts_normal(triangle_verts t) {
    const float3 e0 = t.v1 - t.v0;
    const float3 e1 = t.v2 - t.v0;
    return normalize(cross(e0, e1));
}

float3 triangle_normal(triangle triangle, const global float3 * vertices);
float3 triangle_normal(triangle triangle, const global float3 * vertices) {
    const triangle_verts t = {
        vertices[triangle.v0], vertices[triangle.v1], vertices[triangle.v2]};
    return triangle_verts_normal(t);
}

float3 reflect(float3 normal, float3 direction);
float3 reflect(float3 normal, float3 direction) {
    return direction - (normal * 2 * dot(direction, normal));
}

ray ray_reflect(ray r, float3 normal, float3 intersection);
ray ray_reflect(ray r, float3 normal, float3 intersection) {
    return (ray){intersection, reflect(normal, r.direction)};
}

ray triangle_reflect_at(triangle triangle,
                        const global float3 * vertices,
                        ray r,
                        float3 intersection);
ray triangle_reflect_at(triangle triangle,
                        const global float3 * vertices,
                        ray r,
                        float3 intersection) {
    return ray_reflect(r, triangle_normal(triangle, vertices), intersection);
}

#define INTERSECTION_ACCUMULATOR                                               \
    if (triangle_index != avoid_intersecting_with) {            \
    const triangle tri = triangles[triangle_index]; /* keep on own line */     \
    const triangle_inter inter = triangle_intersection(tri, vertices, r);      \
    if (inter.t && (! ret.inter.t || inter.t < ret.inter.t)) {                 \
        ret.index = triangle_index;                                            \
        ret.inter = inter;                                                     \
    } \
    }

intersection ray_triangle_intersection(ray r,
                                       const global triangle * triangles,
                                       uint numtriangles,
                                       const global float3 * vertices,
                                       uint avoid_intersecting_with);
intersection ray_triangle_intersection(ray r,
                                       const global triangle * triangles,
                                       uint numtriangles,
                                       const global float3 * vertices,
                                       uint avoid_intersecting_with) {
    intersection ret = {};
    for (uint i = 0; i != numtriangles; ++i) {
        const uint triangle_index = i;
        INTERSECTION_ACCUMULATOR
    }
    return ret;
}

intersection ray_triangle_group_intersection(ray r,
                                             const global triangle * triangles,
                                             const global uint * indices,
                                             uint numindices,
                                             const global float3 * vertices,
                                             uint avoid_intersecting_with);
intersection ray_triangle_group_intersection(ray r,
                                             const global triangle * triangles,
                                             const global uint * indices,
                                             uint numindices,
                                             const global float3 * vertices,
                                             uint avoid_intersecting_with) {
    intersection ret = {};
    for (uint i = 0; i != numindices; ++i) {
        const uint triangle_index = indices[i];
        INTERSECTION_ACCUMULATOR
    }
    return ret;
}

float3 get_direction(float3 from, float3 to);
float3 get_direction(float3 from, float3 to) {
    return normalize(to - from);
}

bool line_segment_sphere_intersection(float3 p1, float3 p2, float3 sc, float r);
bool line_segment_sphere_intersection(float3 p1, float3 p2, float3 sc, float r) {
    const float3 diff = p2 - p1;
    const float u = dot(sc - p1, diff) / dot(diff, diff);
    if (u < 0 || 1 < u) {
        return false;
    }
    const float3 closest = p1 + u * diff - sc;
    return dot(closest, closest) < r * r;
}

)"};
}  // namespace cl_sources
