#pragma once

//  please only include in .cpp files

#include "common/cl/scene_structs.h"

#include <string>

namespace cl_sources {
constexpr const char* geometry(R"(

#ifndef GEOMETRY_HEADER__
#define GEOMETRY_HEADER__

typedef struct {
    float3 position;
    float3 direction;
} Ray;

typedef struct {
    ulong primitive;
    float distance;
    char intersects;
} Intersection;

#define EPSILON (0.0001f)
float triangle_vert_intersection(TriangleVerts v, Ray ray);
float triangle_vert_intersection(TriangleVerts v, Ray ray) {
    float3 e0 = v.v1 - v.v0;
    float3 e1 = v.v2 - v.v0;

    float3 pvec = cross(ray.direction, e1);
    float det = dot(e0, pvec);

    if (-EPSILON < det && det < EPSILON)
        return 0.0f;

    float invdet = 1.0f / det;
    float3 tvec = ray.position - v.v0;
    float ucomp = invdet * dot(tvec, pvec);

    if (ucomp < 0.0f || 1.0f < ucomp)
        return 0.0f;

    float3 qvec = cross(tvec, e0);
    float vcomp = invdet * dot(ray.direction, qvec);

    if (vcomp < 0.0f || 1.0f < vcomp + ucomp)
        return 0.0f;

    return invdet * dot(e1, qvec);
}

float triangle_intersection(Triangle triangle,
                            const global float3 * vertices,
                            Ray ray);
float triangle_intersection(Triangle triangle,
                            const global float3 * vertices,
                            Ray ray) {
    const TriangleVerts v = {
        vertices[triangle.v0], vertices[triangle.v1], vertices[triangle.v2]};
    return triangle_vert_intersection(v, ray);
}

float3 triangle_verts_normal(TriangleVerts t);
float3 triangle_verts_normal(TriangleVerts t) {
    const float3 e0 = t.v1 - t.v0;
    const float3 e1 = t.v2 - t.v0;
    return normalize(cross(e0, e1));
}

float3 triangle_normal(Triangle triangle, const global float3 * vertices);
float3 triangle_normal(Triangle triangle, const global float3 * vertices) {
    const TriangleVerts t = {
        vertices[triangle.v0], vertices[triangle.v1], vertices[triangle.v2]};
    return triangle_verts_normal(t);
}

float3 reflect(float3 normal, float3 direction);
float3 reflect(float3 normal, float3 direction) {
    return direction - (normal * 2 * dot(direction, normal));
}

Ray ray_reflect(Ray ray, float3 normal, float3 intersection);
Ray ray_reflect(Ray ray, float3 normal, float3 intersection) {
    return (Ray){intersection, reflect(normal, ray.direction)};
}

Ray triangle_reflect_at(Triangle triangle,
                        const global float3 * vertices,
                        Ray ray,
                        float3 intersection);
Ray triangle_reflect_at(Triangle triangle,
                        const global float3 * vertices,
                        Ray ray,
                        float3 intersection) {
    return ray_reflect(ray, triangle_normal(triangle, vertices), intersection);
}

#define INTERSECTION_ACCUMULATOR                                               \
    const Triangle tri = triangles[triangle_index]; /* keep on own line */     \
    const float distance = triangle_intersection(tri, vertices, ray);          \
    if (EPSILON < distance && (!ret.intersects || distance < ret.distance)) {  \
        ret.primitive = triangle_index;                                        \
        ret.distance = distance;                                               \
        ret.intersects = true;                                                 \
    }

Intersection ray_triangle_intersection(Ray ray,
                                       const global Triangle * triangles,
                                       ulong numtriangles,
                                       const global float3 * vertices);
Intersection ray_triangle_intersection(Ray ray,
                                       const global Triangle * triangles,
                                       ulong numtriangles,
                                       const global float3 * vertices) {
    Intersection ret = {};
    for (ulong i = 0; i != numtriangles; ++i) {
        const ulong triangle_index = i;
        INTERSECTION_ACCUMULATOR
    }
    return ret;
}

Intersection ray_triangle_group_intersection(Ray ray,
                                             const global Triangle * triangles,
                                             const global uint * indices,
                                             ulong numindices,
                                             const global float3 * vertices);
Intersection ray_triangle_group_intersection(Ray ray,
                                             const global Triangle * triangles,
                                             const global uint * indices,
                                             ulong numindices,
                                             const global float3 * vertices) {
    Intersection ret = {};
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

//----------------------------------------------------------------------------//

struct alignas(1 << 4) ray final {
    cl_float3 position;
    cl_float3 direction;
};

constexpr auto to_tuple(const ray& x) {
    return std::tie(x.position, x.direction);
}

constexpr bool operator==(const ray& a, const ray& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const ray& a, const ray& b) { return !(a == b); }

//----------------------------------------------------------------------------//

struct alignas(1 << 3) intersection final {
    cl_ulong primitive;
    cl_float distance;
    cl_char intersects;
};

constexpr auto to_tuple(const intersection& x) {
    return std::tie(x.primitive, x.distance, x.intersects);
}

constexpr bool operator==(const intersection& a, const intersection& b) {
    return to_tuple(a) == to_tuple(b);
}

constexpr bool operator!=(const intersection& a, const intersection& b) {
    return !(a == b);
}
