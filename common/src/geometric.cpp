#include "common/geometric.h"

#include "common/stl_wrappers.h"
#include "common/string_builder.h"
#include "common/triangle.h"

namespace {
const auto EPSILON = 0.0001;
}  // namespace

namespace geo {

Intersects triangle_intersection(const TriangleVec3& tri, const Ray& ray) {
    auto e0 = tri[1] - tri[0];
    auto e1 = tri[2] - tri[0];

    auto pvec = glm::cross(ray.get_direction(), e1);
    auto det = glm::dot(e0, pvec);

    if (-EPSILON < det && det < EPSILON) {
        return Intersects();
    }

    auto invdet = 1 / det;
    auto tvec = ray.get_position() - tri[0];
    auto ucomp = invdet * glm::dot(tvec, pvec);

    if (ucomp < 0 || 1 < ucomp) {
        return Intersects();
    }

    auto qvec = glm::cross(tvec, e0);
    auto vcomp = invdet * glm::dot(ray.get_direction(), qvec);

    if (vcomp < 0 || 1 < vcomp + ucomp) {
        return Intersects();
    }

    auto dist = invdet * glm::dot(e1, qvec);

    if (dist < 0) {
        return Intersects();
    }

    return Intersects(dist);
}

TriangleVec3 to_triangle_vec3f(const Triangle& tri,
                               const std::vector<glm::vec3>& vertices) {
    const auto v0 = vertices[tri.v0];
    const auto v1 = vertices[tri.v1];
    const auto v2 = vertices[tri.v2];
    return TriangleVec3{{v0, v1, v2}};
}

Intersects triangle_intersection(const Triangle& tri,
                                 const std::vector<glm::vec3>& vertices,
                                 const Ray& ray) {
    return triangle_intersection(to_triangle_vec3f(tri, vertices), ray);
}

Intersection ray_triangle_intersection(const Ray& ray,
                                       const std::vector<int>& triangle_indices,
                                       const std::vector<Triangle>& triangles,
                                       const std::vector<glm::vec3>& vertices) {
    Intersection ret;

    for (const auto& i : triangle_indices) {
        auto inter = triangle_intersection(triangles[i], vertices, ray);
        if (inter.get_intersects() &&
            ((!ret.get_intersects()) ||
             (ret.get_intersects() &&
              inter.get_distance() < ret.get_distance()))) {
            ret = Intersection(inter.get_distance(), i);
        }
    }

    return ret;
}

Intersection ray_triangle_intersection(const Ray& ray,
                                       const std::vector<Triangle>& triangles,
                                       const std::vector<glm::vec3>& vertices) {
    std::vector<int> triangle_indices(triangles.size());
    proc::iota(triangle_indices, 0);
    return ray_triangle_intersection(
            ray, triangle_indices, triangles, vertices);
}

bool point_intersection(const glm::vec3& begin,
                        const glm::vec3& point,
                        const std::vector<Triangle>& triangles,
                        const std::vector<glm::vec3>& vertices) {
    auto begin_to_point = point - begin;
    auto mag = glm::length(begin_to_point);
    auto direction = glm::normalize(begin_to_point);

    Ray to_point(begin, direction);

    auto inter = ray_triangle_intersection(to_point, triangles, vertices);

    return (!inter.get_intersects()) || inter.get_distance() > mag;
}

//  adapted from
//  http://www.geometrictools.com/GTEngine/Include/Mathematics/GteDistPointTriangleExact.h
float point_triangle_distance_squared(const TriangleVec3& triangle,
                                      const glm::vec3& point) {
    //  do I hate this? yes
    //  am I going to do anything about it? it works
    const auto diff = point - triangle[0];
    const auto e0 = triangle[1] - triangle[0];
    const auto e1 = triangle[2] - triangle[0];
    const auto a00 = glm::dot(e0, e0);
    const auto a01 = glm::dot(e0, e1);
    const auto a11 = glm::dot(e1, e1);
    const auto b0 = -glm::dot(diff, e0);
    const auto b1 = -glm::dot(diff, e1);
    const auto det = a00 * a11 - a01 * a01;

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
            const auto invDet = 1 / det;
            t0 *= invDet;
            t1 *= invDet;
        }
    } else {
        if (t0 < 0) {
            const auto tmp0 = a01 + b0;
            const auto tmp1 = a11 + b1;
            if (tmp0 < tmp1) {
                const auto numer = tmp1 - tmp0;
                const auto denom = a00 - 2 * a01 + a11;
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
            const auto tmp0 = a01 + b1;
            const auto tmp1 = a00 + b0;
            if (tmp0 < tmp1) {
                const auto numer = tmp1 - tmp0;
                const auto denom = a00 - 2 * a01 + a11;
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
            const auto numer = a11 + b1 - a01 - b0;
            if (numer <= 0) {
                t0 = 0;
                t1 = 1;
            } else {
                const auto denom = a00 - 2 * a01 + a11;
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

    const auto closest = triangle[0] + e0 * t0 + e1 * t1;
    const auto d = point - closest;
    return glm::dot(d, d);
}

float point_triangle_distance_squared(const Triangle& tri,
                                      const std::vector<glm::vec3>& vertices,
                                      const glm::vec3& point) {
    return point_triangle_distance_squared(to_triangle_vec3f(tri, vertices),
                                           point);
}
}  // namespace geo
