#pragma once

#include "triangle_vec.h"

#include "glm/glm.hpp"

namespace geo {

class Ray {
public:
    explicit Ray(const glm::vec3& position = glm::vec3(),
                 const glm::vec3& direction = glm::vec3());
    glm::vec3 position;
    glm::vec3 direction;
};

class Intersects {
public:
    Intersects() = default;
    explicit Intersects(float distance);

    virtual ~Intersects() noexcept = default;
    Intersects(Intersects&&) noexcept = default;
    Intersects& operator=(Intersects&&) noexcept = default;
    Intersects(const Intersects&) noexcept = default;
    Intersects& operator=(const Intersects&) noexcept = default;

    bool intersects{false};
    float distance{0};
};

class Intersection : public Intersects {
public:
    Intersection() = default;
    Intersection(float distance, int index);

    int index{0};
};

TriangleVec3 to_triangle_vec3f(const Triangle& tri,
                               const std::vector<glm::vec3>& vertices);

Intersects triangle_intersection(const TriangleVec3& tri, const Ray& ray);

Intersects triangle_intersection(const Triangle& tri,
                                 const std::vector<glm::vec3>& vertices,
                                 const Ray& ray);

Intersection ray_triangle_intersection(const Ray& ray,
                                       const std::vector<int>& triangle_indices,
                                       const std::vector<Triangle>& triangles,
                                       const std::vector<glm::vec3>& vertices);

Intersection ray_triangle_intersection(const Ray& ray,
                                       const std::vector<Triangle>& triangles,
                                       const std::vector<glm::vec3>& vertices);

bool point_intersection(const glm::vec3& begin,
                        const glm::vec3& point,
                        const std::vector<Triangle>& triangles,
                        const std::vector<glm::vec3>& vertices);

float point_triangle_distance_squared(const TriangleVec3& triangle,
                                      const glm::vec3& point);

float point_triangle_distance_squared(const Triangle& tri,
                                      const std::vector<glm::vec3>& vertices,
                                      const glm::vec3& point);
}  // namespace geo
