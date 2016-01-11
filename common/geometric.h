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
    Intersects();
    Intersects(float distance);

    bool intersects;
    float distance;
};

std::ostream& operator<<(std::ostream& strm, const Intersects& obj);

Intersects triangle_intersection(const TriangleVec3f& tri, const Ray& ray);

Intersects triangle_intersection(const Triangle& tri,
                                 const std::vector<Vec3f>& vertices,
                                 const Ray& ray);

Intersects ray_triangle_intersection(const Ray& ray,
                                     const std::vector<Triangle>& triangles,
                                     const std::vector<Vec3f>& vertices);

bool point_intersection(const Vec3f& begin,
                        const Vec3f& point,
                        const std::vector<Triangle>& triangles,
                        const std::vector<Vec3f>& vertices);
};
