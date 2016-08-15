#pragma once

#include "common/cl/geometry_structs.h"

namespace cl_sources {
const std::string geometry(std::string{} + cl_representation_v<ray> +
                           cl_representation_v<triangle_inter> +
                           cl_representation_v<intersection> + R"(
#ifndef GEOMETRY_HEADER__
#define GEOMETRY_HEADER__

#define EPSILON (0.0001f)

triangle_inter triangle_vert_intersection(triangle_verts verts, ray r);
triangle_inter triangle_vert_intersection(triangle_verts verts, ray r) {
    const float3 e0 = verts.v1 - verts.v0;
    const float3 e1 = verts.v2 - verts.v0;

    const float3 pvec = cross(r.direction, e1);
    const float det = dot(e0, pvec);

    if (-EPSILON < det && det < EPSILON) {
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

    if (t < 0) {
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
    const triangle tri = triangles[triangle_index]; /* keep on own line */     \
    const triangle_inter inter = triangle_intersection(tri, vertices, r);      \
    if (EPSILON < inter.t && (! ret.inter.t || inter.t < ret.inter.t)) {       \
        ret.index = triangle_index;                                            \
        ret.inter = inter;                                                     \
    }

intersection ray_triangle_intersection(ray r,
                                       const global triangle * triangles,
                                       ulong numtriangles,
                                       const global float3 * vertices);
intersection ray_triangle_intersection(ray r,
                                       const global triangle * triangles,
                                       ulong numtriangles,
                                       const global float3 * vertices) {
    intersection ret = {};
    for (ulong i = 0; i != numtriangles; ++i) {
        const ulong triangle_index = i;
        INTERSECTION_ACCUMULATOR
    }
    return ret;
}

intersection ray_triangle_group_intersection(ray r,
                                             const global triangle * triangles,
                                             const global uint * indices,
                                             ulong numindices,
                                             const global float3 * vertices);
intersection ray_triangle_group_intersection(ray r,
                                             const global triangle * triangles,
                                             const global uint * indices,
                                             ulong numindices,
                                             const global float3 * vertices) {
    intersection ret = {};
    for (ulong i = 0; i != numindices; ++i) {
        const uint triangle_index = indices[i];
        INTERSECTION_ACCUMULATOR
    }
    return ret;
}

float3 get_direction(float3 from, float3 to);
float3 get_direction(float3 from, float3 to) {
    return normalize(to - from);
}

#endif

)");
}  // namespace cl_sources
