#pragma once

#include "common/almost_equal.h"
#include "common/cl/geometry_structs.h"
#include "common/conversions.h"
#include "common/geo/triangle_vec.h"

#include "utilities/aligned/vector.h"

#include "glm/glm.hpp"

#include <experimental/optional>
#include <numeric>

namespace geo {

/// I would do this with a struct, but rays have an invariant:
//      the direction is a normalized vector
class ray final {
public:
    inline ray(const glm::vec3& position, const glm::vec3& direction)
            : position(position)
            , direction(glm::normalize(direction)) {}

    constexpr glm::vec3 get_position() const { return position; }
    constexpr glm::vec3 get_direction() const { return direction; }

private:
    glm::vec3 position;
    glm::vec3 direction;
};

////////////////////////////////////////////////////////////////////////////////

std::experimental::optional<triangle_inter> triangle_intersection(
        const triangle_vec3& tri, const ray& ray, size_t ulp = 10);

template <typename t>
std::experimental::optional<triangle_inter> triangle_intersection(
        const triangle& tri,
        const aligned::vector<t>& vertices,
        const ray& ray) {
    return triangle_intersection(get_triangle_vec3(tri, vertices), ray);
}

////////////////////////////////////////////////////////////////////////////////

template <typename t>
std::experimental::optional<intersection> intersection_accumulator(
        const ray& ray,
        size_t triangle_index,
        const aligned::vector<triangle>& triangles,
        const aligned::vector<t> vertices,
        const std::experimental::optional<intersection>& current,
        size_t to_ignore = ~size_t{0}) {
    if (triangle_index == to_ignore) {
        return current;
    }
    const auto i =
            triangle_intersection(triangles[triangle_index], vertices, ray);
    return (i && (!current || i->t < current->inter.t))
                   ? intersection{*i, static_cast<cl_uint>(triangle_index)}
                   : current;
}

template <typename t>
std::experimental::optional<intersection> ray_triangle_intersection(
        const ray& ray,
        const aligned::vector<size_t>& triangle_indices,
        const aligned::vector<triangle>& triangles,
        const aligned::vector<t>& vertices,
        size_t to_ignore = ~size_t{0}) {
    return std::accumulate(
            begin(triangle_indices),
            end(triangle_indices),
            std::experimental::optional<intersection>{},
            [&](const auto& i, const auto& j) {
                return intersection_accumulator(
                        ray, j, triangles, vertices, i, to_ignore);
            });
}

template <typename t>
std::experimental::optional<intersection> ray_triangle_intersection(
        const ray& ray,
        const aligned::vector<triangle>& triangles,
        const aligned::vector<t>& vertices,
        size_t to_ignore = ~size_t{0}) {
    std::experimental::optional<intersection> ret;
    for (auto i = 0u; i != triangles.size(); ++i) {
        ret = intersection_accumulator(
                ray, i, triangles, vertices, ret, to_ignore);
    }
    return ret;
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

constexpr ray convert(const geo::ray& r) {
    return ray{to_cl_float3(r.get_position()), to_cl_float3(r.get_direction())};
}

inline geo::ray convert(const ray& r) {
    return geo::ray{to_vec3(r.position), to_vec3(r.direction)};
}

constexpr triangle_verts convert(const geo::triangle_vec3& t) {
    return triangle_verts{
            to_cl_float3(t[0]), to_cl_float3(t[1]), to_cl_float3(t[2])};
}

constexpr geo::triangle_vec3 convert(const triangle_verts& t) {
    return geo::triangle_vec3{{to_vec3(t.v0), to_vec3(t.v1), to_vec3(t.v2)}};
}
