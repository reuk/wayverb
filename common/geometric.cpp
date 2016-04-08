#include "geometric.h"

#include "stl_wrappers.h"
#include "string_builder.h"
#include "triangle.h"

static const auto EPSILON = 0.0001;

namespace geo {

Ray::Ray(const Vec3f& position, const Vec3f& direction)
        : position(position)
        , direction(direction) {
}

Intersects::Intersects(float distance)
        : intersects(true)
        , distance(distance) {
}

Intersection::Intersection(float distance, int index)
        : Intersects(distance)
        , index(index) {
}

Intersects triangle_intersection(const TriangleVec3f& tri, const Ray& ray) {
    auto e0 = tri[1] - tri[0];
    auto e1 = tri[2] - tri[0];

    auto pvec = ray.direction.cross(e1);
    auto det = e0.dot(pvec);

    if (-EPSILON < det && det < EPSILON) {
        return Intersects();
    }

    auto invdet = 1 / det;
    auto tvec = ray.position - tri[0];
    auto ucomp = invdet * tvec.dot(pvec);

    if (ucomp < 0 || 1 < ucomp) {
        return Intersects();
    }

    auto qvec = tvec.cross(e0);
    auto vcomp = invdet * ray.direction.dot(qvec);

    if (vcomp < 0 || 1 < vcomp + ucomp) {
        return Intersects();
    }

    auto dist = invdet * e1.dot(qvec);

    if (dist < 0) {
        return Intersects();
    }

    return Intersects(dist);
}

TriangleVec3f to_triangle_vec3f(const Triangle& tri,
                                const std::vector<Vec3f>& vertices) {
    const auto v0 = vertices[tri.v0];
    const auto v1 = vertices[tri.v1];
    const auto v2 = vertices[tri.v2];
    return TriangleVec3f{{v0, v1, v2}};
}

Intersects triangle_intersection(const Triangle& tri,
                                 const std::vector<Vec3f>& vertices,
                                 const Ray& ray) {
    return triangle_intersection(to_triangle_vec3f(tri, vertices), ray);
}

Intersection ray_triangle_intersection(const Ray& ray,
                                       const std::vector<int>& triangle_indices,
                                       const std::vector<Triangle>& triangles,
                                       const std::vector<Vec3f>& vertices) {
    Intersection ret;

    for (const auto& i : triangle_indices) {
        auto inter = triangle_intersection(triangles[i], vertices, ray);
        if (inter.intersects &&
            ((!ret.intersects) ||
             (ret.intersects && inter.distance < ret.distance))) {
            ret = Intersection(inter.distance, i);
        }
    }

    return ret;
}

Intersection ray_triangle_intersection(const Ray& ray,
                                       const std::vector<Triangle>& triangles,
                                       const std::vector<Vec3f>& vertices) {
    std::vector<int> triangle_indices(triangles.size());
    proc::iota(triangle_indices, 0);
    return ray_triangle_intersection(
        ray, triangle_indices, triangles, vertices);
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

std::ostream& operator<<(std::ostream& os, const Intersects& obj) {
    Bracketer bracketer(os);
    return to_stream(
        os, "intersects: ", obj.intersects, "  distance: ", obj.distance, "  ");
}

//  adapted from
//  http://www.geometrictools.com/GTEngine/Include/Mathematics/GteDistPointTriangleExact.h
float point_triangle_distance_squared(const TriangleVec3f& triangle,
                                      const Vec3f& point) {
    //  do I hate this? yes
    //  am I going to do anything about it? it works
    auto diff = point - triangle[0];
    auto e0 = triangle[1] - triangle[0];
    auto e1 = triangle[2] - triangle[0];
    auto a00 = e0.dot(e0);
    auto a01 = e0.dot(e1);
    auto a11 = e1.dot(e1);
    auto b0 = -(diff.dot(e0));
    auto b1 = -(diff.dot(e1));
    auto det = a00 * a11 - a01 * a01;
    auto t0 = a01 * b1 - a11 * b0;
    auto t1 = a01 * b0 - a00 * b1;

    if (t0 + t1 <= det) {
        if (t0 < 0) {
            if (t1 < 0) {
                if (b0 < 0) {
                    t1 = 0;
                    if (a00 <= -b0) {
                        t0 = 1;
                    } else {
                        t0 = -b0 / a00;
                    }
                } else {
                    t0 = 0;
                    if (0 <= b1) {
                        t1 = 0;
                    } else if (a11 <= -b1) {
                        t1 = 1;
                    } else {
                        t1 = -b1 / a11;
                    }
                }
            } else {
                t0 = 0;
                if (0 <= b1) {
                    t1 = 0;
                } else if (a11 <= -b1) {
                    t1 = 1;
                } else {
                    t1 = -b1 / a11;
                }
            }
        } else if (t1 < 0) {
            t1 = 0;
            if (0 <= b0) {
                t0 = 0;
            } else if (a00 <= -b0) {
                t0 = 1;
            } else {
                t0 = -b0 / a00;
            }
        } else {
            auto invDet = 1 / det;
            t0 *= invDet;
            t1 *= invDet;
        }
    } else {
        if (t0 < 0) {
            auto tmp0 = a01 + b0;
            auto tmp1 = a11 + b1;
            if (tmp0 < tmp1) {
                auto numer = tmp1 - tmp0;
                auto denom = a00 - 2 * a01 + a11;
                if (denom <= numer) {
                    t0 = 1;
                    t1 = 0;
                } else {
                    t0 = numer / denom;
                    t1 = 1 - t0;
                }
            } else {
                t0 = 0;
                if (tmp1 <= 0) {
                    t1 = 1;
                } else if (0 <= b1) {
                    t1 = 0;
                } else {
                    t1 = -b1 / a11;
                }
            }
        } else if (t1 < 0) {
            auto tmp0 = a01 + b1;
            auto tmp1 = a00 + b0;
            if (tmp0 < tmp1) {
                auto numer = tmp1 - tmp0;
                auto denom = a00 - 2 * a01 + a11;
                if (denom <= numer) {
                    t1 = 1;
                    t0 = 0;
                } else {
                    t1 = numer / denom;
                    t0 = 1 - t1;
                }
            } else {
                t1 = 0;
                if (tmp1 <= 0) {
                    t0 = 1;
                } else if (0 <= b0) {
                    t0 = 0;
                } else {
                    t0 = -b0 / a00;
                }
            }
        } else {
            auto numer = a11 + b1 - a01 - b0;
            if (numer <= 0) {
                t0 = 0;
                t1 = 1;
            } else {
                auto denom = a00 - 2 * a01 + a11;
                if (denom <= numer) {
                    t0 = 1;
                    t1 = 0;
                } else {
                    t0 = numer / denom;
                    t1 = 1 - t0;
                }
            }
        }
    }

    auto closest = triangle[0] + e0 * t0 + e1 * t1;
    diff = point - closest;
    return diff.dot(diff);
}

float point_triangle_distance_squared(const Triangle& tri,
                                      const std::vector<Vec3f>& vertices,
                                      const Vec3f& point) {
    return point_triangle_distance_squared(to_triangle_vec3f(tri, vertices),
                                           point);
}
}  // namespace geo
