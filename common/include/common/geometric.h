#pragma once

#include "common/aligned/vector.h"
#include "common/triangle.h"
#include "common/triangle_vec.h"

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

using intersects = std::experimental::optional<float>;

namespace detail {
struct inter final {
    float distance;
    size_t index;
};

constexpr bool operator==(const inter& a, const inter& b) {
    return std::tie(a.distance, a.index) == std::tie(b.distance, b.index);
}

constexpr bool operator!=(const inter& a, const inter& b) { return !(a == b); }
}  // namespace detail

using intersection = std::experimental::optional<detail::inter>;

inline auto make_intersection(float distance, size_t index) {
    return intersection(detail::inter{distance, index});
}

intersects triangle_intersection(const triangle_vec3& tri, const ray& ray);

intersects triangle_intersection(const triangle& tri,
                                 const aligned::vector<glm::vec3>& vertices,
                                 const ray& ray);

intersection ray_triangle_intersection(
        const ray& ray,
        const aligned::vector<size_t>& triangle_indices,
        const aligned::vector<triangle>& triangles,
        const aligned::vector<glm::vec3>& vertices);

intersection ray_triangle_intersection(
        const ray& ray,
        const aligned::vector<triangle>& triangles,
        const aligned::vector<glm::vec3>& vertices);

bool point_intersection(const glm::vec3& begin,
                        const glm::vec3& point,
                        const aligned::vector<triangle>& triangles,
                        const aligned::vector<glm::vec3>& vertices);

float point_triangle_distance_squared(const triangle_vec3& triangle,
                                      const glm::vec3& point);

float point_triangle_distance_squared(
        const triangle& tri,
        const aligned::vector<glm::vec3>& vertices,
        const glm::vec3& point);

glm::vec3 normal(const triangle_vec3& t);

glm::vec3 mirror(const glm::vec3& p, const triangle_vec3& t);
triangle_vec3 mirror(const triangle_vec3& in, const triangle_vec3& t);

}  // namespace geo
