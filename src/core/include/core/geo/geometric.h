#pragma once

#include "core/cl/geometry_structs.h"
#include "core/cl/triangle.h"

#include "glm/fwd.hpp"

#include <experimental/optional>
#include <numeric>

namespace wayverb {
namespace core {
namespace geo {

struct triangle_vec3;

/// I would do this with a struct, but rays have an invariant:
//      the direction is a normalized vector
class ray final {
public:
    ray(const glm::vec3& position, const glm::vec3& direction);

    glm::vec3 get_position() const;
    glm::vec3 get_direction() const;

private:
    glm::vec3 position_;
    glm::vec3 direction_;
};

////////////////////////////////////////////////////////////////////////////////

std::experimental::optional<triangle_inter> triangle_intersection(
        const triangle_vec3& tri, const ray& ray, size_t ulp = 10);

template <typename T>
std::experimental::optional<triangle_inter> triangle_intersection(
        const triangle& tri, const T* v, const ray& ray);

////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::experimental::optional<intersection> intersection_accumulator(
        const ray& ray,
        size_t triangle_index,
        const triangle* triangles,
        const T* vertices,
        const std::experimental::optional<intersection>& current,
        size_t to_ignore = ~size_t{0});

template <typename T>
std::experimental::optional<intersection> ray_triangle_intersection(
        const ray& ray,
        const size_t* triangle_indices,
        size_t num_triangle_indices,
        const triangle* triangles,
        const T* vertices,
        size_t to_ignore = ~size_t{0});

template <typename T>
std::experimental::optional<intersection> ray_triangle_intersection(
        const ray& ray,
        const triangle* triangles,
        size_t num_triangles,
        const T* vertices,
        size_t to_ignore = ~size_t{0});

bool point_intersection(const glm::vec3& begin,
                        const glm::vec3& point,
                        const triangle* triangles,
                        size_t num_triangles,
                        const glm::vec3* vertices,
                        size_t num_vertices);

float point_triangle_distance_squared(const triangle_vec3& triangle,
                                      const glm::vec3& point);

float point_triangle_distance_squared(const triangle& tri,
                                      const glm::vec3* vertices,
                                      size_t num_vertices,
                                      const glm::vec3& point);

glm::vec3 normal(const triangle_vec3& t);

glm::vec3 mirror(const glm::vec3& p, const triangle_vec3& t);
triangle_vec3 mirror(const triangle_vec3& in, const triangle_vec3& t);

}  // namespace geo

ray convert(const geo::ray& r);

geo::ray convert(const ray& r);

triangle_verts convert(const geo::triangle_vec3& t);

geo::triangle_vec3 convert(const triangle_verts& t);

}  // namespace core
}  // namespace wayverb
