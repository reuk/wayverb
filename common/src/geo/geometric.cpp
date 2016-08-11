#include "common/geo/geometric.h"

#include "common/aligned/vector.h"
#include "common/stl_wrappers.h"
#include "common/string_builder.h"
#include "common/triangle.h"

namespace {
const auto EPSILON = 0.0001;
}  // namespace

namespace geo {

ray::ray(const glm::vec3& position, const glm::vec3& direction)
        : position(position)
        , direction(glm::normalize(direction)) {}

glm::vec3 ray::get_position() const { return position; }
glm::vec3 ray::get_direction() const { return direction; }

intersects triangle_intersection(const triangle_vec3& tri, const ray& ray) {
    auto e0 = tri[1] - tri[0];
    auto e1 = tri[2] - tri[0];

    auto pvec = glm::cross(ray.get_direction(), e1);
    auto det = glm::dot(e0, pvec);

    if (-EPSILON < det && det < EPSILON) {
        return intersects();
    }

    auto invdet = 1 / det;
    auto tvec = ray.get_position() - tri[0];
    auto ucomp = invdet * glm::dot(tvec, pvec);

    if (ucomp < 0 || 1 < ucomp) {
        return intersects();
    }

    auto qvec = glm::cross(tvec, e0);
    auto vcomp = invdet * glm::dot(ray.get_direction(), qvec);

    if (vcomp < 0 || 1 < vcomp + ucomp) {
        return intersects();
    }

    auto dist = invdet * glm::dot(e1, qvec);

    if (dist < 0) {
        return intersects();
    }

    return intersects(dist);
}

bool point_intersection(const glm::vec3& begin,
                        const glm::vec3& point,
                        const aligned::vector<triangle>& triangles,
                        const aligned::vector<glm::vec3>& vertices) {
    auto begin_to_point = point - begin;
    auto mag = glm::length(begin_to_point);
    auto direction = glm::normalize(begin_to_point);

    ray to_point(begin, direction);

    auto inter = ray_triangle_intersection(to_point, triangles, vertices);

    return !inter || inter->distance > mag;
}

//  adapted from
//  http://www.geometrictools.com/GTEngine/Include/Mathematics/GteDistPointtriangleExact.h
float point_triangle_distance_squared(const triangle_vec3& triangle,
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

glm::vec3 normal(const triangle_vec3& t) {
    return glm::normalize(glm::cross(t[1] - t[0], t[2] - t[0]));
}

glm::vec3 mirror(const glm::vec3& p, const triangle_vec3& t) {
    const auto n = normal(t);
    const auto ret = p - n * glm::dot(n, p - t[0]) * 2.0f;
    return ret;
}

triangle_vec3 mirror(const triangle_vec3& in, const triangle_vec3& t) {
    return triangle_vec3{{mirror(in[0], t), mirror(in[1], t), mirror(in[2], t)}};
}

}  // namespace geo
