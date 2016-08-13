#pragma once

#include "common/aligned/vector.h"
#include "common/almost_equal.h"
#include "common/cl/scene_structs.h"
#include "common/geo/triangle_vec.h"
#include "common/stl_wrappers.h"

#include "glm/glm.hpp"

#include <experimental/optional>

namespace geo {

/// I would do this with a struct, but rays have an invariant:
//      the direction is a normalized vector
class ray final {
public:
    ray(const glm::vec3& position, const glm::vec3& direction);

    glm::vec3 get_position() const;
    glm::vec3 get_direction() const;

private:
    glm::vec3 position;
    glm::vec3 direction;
};

struct triangle_inter final {
    float t;  //  the distance along the ray to the intersection
    float u;  //  barycentric coordinate u
    float v;  //  barycentric coordinate v
};

constexpr bool is_degenerate(const triangle_inter& i) {
    const auto ulp = 10;
    return almost_equal(i.u, 0.0f, ulp) || almost_equal(i.v, 0.0f, ulp) ||
           almost_equal(i.u + i.v, 1.0f, ulp);
}

constexpr bool operator==(const triangle_inter& a, const triangle_inter& b) {
    return std::tie(a.t, a.u, a.v) == std::tie(b.t, b.u, b.v);
}

constexpr bool operator!=(const triangle_inter& a, const triangle_inter& b) {
    return !(a == b);
}

struct scene_triangle_inter final {
    triangle_inter inter;
    size_t index;
};

constexpr bool operator==(const scene_triangle_inter& a,
                          const scene_triangle_inter& b) {
    return std::tie(a.inter, a.index) == std::tie(b.inter, b.index);
}

constexpr bool operator!=(const scene_triangle_inter& a,
                          const scene_triangle_inter& b) {
    return !(a == b);
}

std::experimental::optional<triangle_inter> triangle_intersection(
        const triangle_vec3& tri, const ray& ray, size_t ulp = 10);

template <typename t>
std::experimental::optional<triangle_inter> triangle_intersection(
        const triangle& tri,
        const aligned::vector<t>& vertices,
        const ray& ray) {
    return triangle_intersection(get_triangle_vec3(tri, vertices), ray);
}

template <typename t>
std::experimental::optional<scene_triangle_inter> ray_triangle_intersection(
        const ray& ray,
        const aligned::vector<size_t>& triangle_indices,
        const aligned::vector<triangle>& triangles,
        const aligned::vector<t>& vertices) {
    return proc::accumulate(
            triangle_indices,
            std::experimental::optional<scene_triangle_inter>{},
            [&](const auto& i, const auto& j) {
                const auto inter =
                        triangle_intersection(triangles[j], vertices, ray);
                return (inter && (!i || (i && inter->t < i->inter.t)))
                               ? scene_triangle_inter{*inter, j}
                               : i;
            });
}

template <typename t>
std::experimental::optional<scene_triangle_inter> ray_triangle_intersection(
        const ray& ray,
        const aligned::vector<triangle>& triangles,
        const aligned::vector<t>& vertices) {
    aligned::vector<size_t> triangle_indices(triangles.size());
    proc::iota(triangle_indices, 0);
    return ray_triangle_intersection(
            ray, triangle_indices, triangles, vertices);
}

bool point_intersection(const glm::vec3& begin,
                        const glm::vec3& point,
                        const aligned::vector<triangle>& triangles,
                        const aligned::vector<glm::vec3>& vertices);

float point_triangle_distance_squared(const triangle_vec3& triangle,
                                      const glm::vec3& point);

template <typename t>
float point_triangle_distance_squared(const triangle& tri,
                                      const aligned::vector<t>& vertices,
                                      const glm::vec3& point) {
    return point_triangle_distance_squared(get_triangle_vec3(tri, vertices),
                                           point);
}

glm::vec3 normal(const triangle_vec3& t);

glm::vec3 mirror(const glm::vec3& p, const triangle_vec3& t);
triangle_vec3 mirror(const triangle_vec3& in, const triangle_vec3& t);

}  // namespace geo
