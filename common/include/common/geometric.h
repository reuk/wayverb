#pragma once

#include "common/aligned/vector.h"

#include "triangle_vec.h"

#include "glm/glm.hpp"

#include <experimental/optional>

namespace geo {

class Ray final {
public:
    constexpr explicit Ray(const glm::vec3& position  = glm::vec3(),
                           const glm::vec3& direction = glm::vec3())
            : position(position)
            , direction(direction) {}

    constexpr glm::vec3 get_position() const { return position; }
    constexpr glm::vec3 get_direction() const { return direction; }

private:
    glm::vec3 position;
    glm::vec3 direction;
};

using Intersects = std::experimental::optional<float>;

struct Inter {
    float distance;
    size_t index;
};

constexpr bool operator==(const Inter& a, const Inter& b) {
    return std::tie(a.distance, a.index) == std::tie(b.distance, b.index);
}

using Intersection = std::experimental::optional<Inter>;

TriangleVec3 to_triangle_vec3f(const Triangle& tri,
                               const aligned::vector<glm::vec3>& vertices);

Intersects triangle_intersection(const TriangleVec3& tri, const Ray& ray);

Intersects triangle_intersection(const Triangle& tri,
                                 const aligned::vector<glm::vec3>& vertices,
                                 const Ray& ray);

Intersection ray_triangle_intersection(
        const Ray& ray,
        const aligned::vector<size_t>& triangle_indices,
        const aligned::vector<Triangle>& triangles,
        const aligned::vector<glm::vec3>& vertices);

Intersection ray_triangle_intersection(
        const Ray& ray,
        const aligned::vector<Triangle>& triangles,
        const aligned::vector<glm::vec3>& vertices);

bool point_intersection(const glm::vec3& begin,
                        const glm::vec3& point,
                        const aligned::vector<Triangle>& triangles,
                        const aligned::vector<glm::vec3>& vertices);

float point_triangle_distance_squared(const TriangleVec3& triangle,
                                      const glm::vec3& point);

float point_triangle_distance_squared(
        const Triangle& tri,
        const aligned::vector<glm::vec3>& vertices,
        const glm::vec3& point);
}  // namespace geo
