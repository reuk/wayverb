#include "geometric.h"

static const auto EPSILON = 0.0001;

namespace geo {

Ray::Ray(const Vec3f& position, const Vec3f& direction)
        : position(position)
        , direction(direction) {
}

Intersects::Intersects()
        : intersects(false) {
}

Intersects::Intersects(float distance)
        : intersects(true)
        , distance(distance) {
}

Intersects triangle_intersection(const TriangleVec3f& tri, const Ray& ray) {
    auto e0 = tri[1] - tri[0];
    auto e1 = tri[2] - tri[0];

    auto pvec = ray.direction.cross(e1);
    auto det = e0.dot(pvec);

    if (-EPSILON < det && det < EPSILON)
        return Intersects();

    auto invdet = 1 / det;
    auto tvec = ray.position - tri[0];
    auto ucomp = invdet * tvec.dot(pvec);

    if (ucomp < 0 || 1 < ucomp)
        return Intersects();

    auto qvec = tvec.cross(e0);
    auto vcomp = invdet * ray.direction.dot(qvec);

    if (vcomp < 0 || 1 < vcomp + ucomp)
        return Intersects();

    auto dist = invdet * e1.dot(qvec);

    if (dist < 0)
        return Intersects();

    return Intersects(dist);
}

Intersects triangle_intersection(const Triangle& tri,
                                 const std::vector<Vec3f>& vertices,
                                 const Ray& ray) {
    const auto v0 = vertices[tri.v0];
    const auto v1 = vertices[tri.v1];
    const auto v2 = vertices[tri.v2];
    return triangle_intersection({{v0, v1, v2}}, ray);
}

Intersects ray_triangle_intersection(const Ray& ray,
                                     const std::vector<Triangle>& triangles,
                                     const std::vector<Vec3f>& vertices) {
    Intersects ret;

    for (const auto& i : triangles) {
        auto inter = triangle_intersection(i, vertices, ray);
        if (inter.intersects &&
            ((!ret.intersects) ||
             (ret.intersects && inter.distance < ret.distance))) {
            ret = Intersects(inter.distance);
        }
    }

    return ret;
}

bool point_intersection(const Vec3f& begin,
                        const Vec3f& point,
                        const std::vector<Triangle>& triangles,
                        const std::vector<Vec3f>& vertices) {
    auto begin_to_point = point - begin;
    auto mag = begin_to_point.mag();
    auto direction = begin_to_point.normalized();

    Ray to_point(begin, direction);

    auto inter = ray_triangle_intersection(to_point, triangles, vertices);

    return (!inter.intersects) || inter.distance > mag;
}

std::ostream& operator<<(std::ostream& strm, const Intersects& obj) {
    return strm << "Intersects {" << obj.intersects << ", " << obj.distance
                << "}";
}
};
