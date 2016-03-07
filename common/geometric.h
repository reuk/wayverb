#pragma once

#include "vec.h"
#include "scene_data.h"

namespace geo {

class Ray {
public:
    Ray(const Vec3f& position = Vec3f(), const Vec3f& direction = Vec3f());
    Vec3f position;
    Vec3f direction;
};

class Intersects {
public:
    Intersects() = default;
    Intersects(float distance);

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

std::ostream& operator<<(std::ostream& strm, const Intersects& obj);

TriangleVec3f to_triangle_vec3f(const Triangle& tri,
                                const std::vector<Vec3f>& vertices);

Intersects triangle_intersection(const TriangleVec3f& tri, const Ray& ray);

Intersects triangle_intersection(const Triangle& tri,
                                 const std::vector<Vec3f>& vertices,
                                 const Ray& ray);

Intersection ray_triangle_intersection(const Ray& ray,
                                       const std::vector<int>& triangle_indices,
                                       const std::vector<Triangle>& triangles,
                                       const std::vector<Vec3f>& vertices);

Intersection ray_triangle_intersection(const Ray& ray,
                                       const std::vector<Triangle>& triangles,
                                       const std::vector<Vec3f>& vertices);

bool point_intersection(const Vec3f& begin,
                        const Vec3f& point,
                        const std::vector<Triangle>& triangles,
                        const std::vector<Vec3f>& vertices);

float point_triangle_distance_squared(const TriangleVec3f& triangle,
                                      const Vec3f& point);

float point_triangle_distance_squared(const Triangle& tri,
                                      const std::vector<Vec3f>& vertices,
                                      const Vec3f& point);
};
