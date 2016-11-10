#include "core/geo/geometric.h"
#include "core/almost_equal.h"
#include "core/conversions.h"
#include "core/geo/triangle_vec.h"

#include "utilities/aligned/vector.h"
#include "utilities/string_builder.h"

namespace wayverb {
namespace core {
namespace geo {

ray::ray(const glm::vec3& position, const glm::vec3& direction)
        : position_{position}
        , direction_{glm::normalize(direction)} {}

glm::vec3 ray::get_position() const { return position_; }
glm::vec3 ray::get_direction() const { return direction_; }

////////////////////////////////////////////////////////////////////////////////

std::experimental::optional<triangle_inter> triangle_intersection(
        const triangle_vec3& tri, const ray& ray, size_t ulp) {
    //  from Fast, Minimum Storage Ray/Triangle Intersection
    //  by Moller and Trumbore
    const auto e0 = tri.s[1] - tri.s[0];
    const auto e1 = tri.s[2] - tri.s[0];

    const auto pvec = glm::cross(ray.get_direction(), e1);
    const auto det = glm::dot(e0, pvec);

    if (almost_equal(det, 0.0f, ulp)) {
        return std::experimental::nullopt;
    }

    const auto invdet = 1 / det;
    const auto tvec = ray.get_position() - tri.s[0];
    const auto u = invdet * glm::dot(tvec, pvec);

    if (u < 0 || 1 < u) {
        return std::experimental::nullopt;
    }

    const auto qvec = glm::cross(tvec, e0);
    const auto v = invdet * glm::dot(ray.get_direction(), qvec);

    if (v < 0 || 1 < v + u) {
        return std::experimental::nullopt;
    }

    const auto t = invdet * glm::dot(e1, qvec);

    if (t < 0 || almost_equal(t, 0.0f, ulp)) {
        return std::experimental::nullopt;
    }

    return triangle_inter{t, u, v};
}

template <typename T>
std::experimental::optional<triangle_inter> triangle_intersection(
        const triangle& tri, const T* v, const ray& ray) {
    return triangle_intersection(get_triangle_vec3(tri, v), ray);
}

template std::experimental::optional<triangle_inter>
triangle_intersection<glm::vec3>(const triangle& tri,
                                 const glm::vec3* v,
                                 const ray& ray);

template std::experimental::optional<triangle_inter>
triangle_intersection<cl_float3>(const triangle& tri,
                                 const cl_float3* v,
                                 const ray& ray);

////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::experimental::optional<intersection> intersection_accumulator(
        const ray& ray,
        size_t triangle_index,
        const triangle* triangles,
        const T* vertices,
        const std::experimental::optional<intersection>& current,
        size_t to_ignore) {
    if (triangle_index == to_ignore) {
        return current;
    }
    const auto i =
            triangle_intersection(triangles[triangle_index], vertices, ray);
    return (i && (!current || i->t < current->inter.t))
                   ? intersection{*i, static_cast<cl_uint>(triangle_index)}
                   : current;
}

template std::experimental::optional<intersection> intersection_accumulator<
        glm::vec3>(const ray& ray,
                   size_t triangle_index,
                   const triangle* triangles,
                   const glm::vec3* vertices,
                   const std::experimental::optional<intersection>& current,
                   size_t to_ignore);

template std::experimental::optional<intersection> intersection_accumulator<
        cl_float3>(const ray& ray,
                   size_t triangle_index,
                   const triangle* triangles,
                   const cl_float3* vertices,
                   const std::experimental::optional<intersection>& current,
                   size_t to_ignore);

////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::experimental::optional<intersection> ray_triangle_intersection(
        const ray& ray,
        const size_t* triangle_indices,
        size_t num_triangle_indices,
        const triangle* triangles,
        const T* vertices,
        size_t to_ignore) {
    return std::accumulate(
            triangle_indices,
            triangle_indices + num_triangle_indices,
            std::experimental::optional<intersection>{},
            [&](const auto& i, const auto& j) {
                return intersection_accumulator(
                        ray, j, triangles, vertices, i, to_ignore);
            });
}

template std::experimental::optional<intersection>
ray_triangle_intersection<glm::vec3>(const ray& ray,
                                     const size_t* triangle_indices,
                                     size_t num_triangle_indices,
                                     const triangle* triangles,
                                     const glm::vec3* vertices,
                                     size_t to_ignore);

template std::experimental::optional<intersection>
ray_triangle_intersection<cl_float3>(const ray& ray,
                                     const size_t* triangle_indices,
                                     size_t num_triangle_indices,
                                     const triangle* triangles,
                                     const cl_float3* vertices,
                                     size_t to_ignore);

////////////////////////////////////////////////////////////////////////////////

template <typename T>
std::experimental::optional<intersection> ray_triangle_intersection(
        const ray& ray,
        const triangle* triangles,
        size_t num_triangles,
        const T* vertices,
        size_t to_ignore) {
    std::experimental::optional<intersection> ret;
    for (auto i = 0u; i != num_triangles; ++i) {
        ret = intersection_accumulator(
                ray, i, triangles, vertices, ret, to_ignore);
    }
    return ret;
}

template std::experimental::optional<intersection>
ray_triangle_intersection<glm::vec3>(const ray& ray,
                                     const triangle* triangles,
                                     size_t num_triangles,
                                     const glm::vec3* vertices,
                                     size_t to_ignore);

template std::experimental::optional<intersection>
ray_triangle_intersection<cl_float3>(const ray& ray,
                                     const triangle* triangles,
                                     size_t num_triangles,
                                     const cl_float3* vertices,
                                     size_t to_ignore);

////////////////////////////////////////////////////////////////////////////////

bool point_intersection(const glm::vec3& begin,
                        const glm::vec3& point,
                        const triangle* triangles,
                        size_t num_triangles,
                        const glm::vec3* vertices,
                        size_t num_vertices) {
    const auto begin_to_point = point - begin;
    const auto mag = glm::length(begin_to_point);
    const auto direction = glm::normalize(begin_to_point);

    const ray to_point(begin, direction);

    const auto ret = ray_triangle_intersection(
            to_point, triangles, num_triangles, vertices, num_vertices);

    return !ret || mag < ret->inter.t;
}

float point_triangle_distance_squared(const triangle& tri,
                                      const glm::vec3* vertices,
                                      size_t num_vertices,
                                      const glm::vec3& point) {
    return point_triangle_distance_squared(get_triangle_vec3(tri, vertices),
                                           point);
}

//  adapted from
//  http://www.geometrictools.com/GTEngine/Include/Mathematics/GteDistPointtriangleExact.h
float point_triangle_distance_squared(const triangle_vec3& triangle,
                                      const glm::vec3& point) {
    //  do I hate this? yes
    //  am I going to do anything about it? it works
    const auto diff = point - triangle.s[0];
    const auto e0 = triangle.s[1] - triangle.s[0];
    const auto e1 = triangle.s[2] - triangle.s[0];
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

    const auto closest = triangle.s[0] + e0 * t0 + e1 * t1;
    const auto d = point - closest;
    return glm::dot(d, d);
}

glm::vec3 normal(const triangle_vec3& t) {
    return glm::normalize(glm::cross(t.s[1] - t.s[0], t.s[2] - t.s[0]));
}

glm::vec3 mirror(const glm::vec3& p, const triangle_vec3& t) {
    const auto n = normal(t);
    const auto ret = p - n * glm::dot(n, p - t.s[0]) * 2.0f;
    return ret;
}

triangle_vec3 mirror(const triangle_vec3& in, const triangle_vec3& t) {
    return triangle_vec3{
            {{mirror(in.s[0], t), mirror(in.s[1], t), mirror(in.s[2], t)}}};
}

}  // namespace geo

////////////////////////////////////////////////////////////////////////////////

ray convert(const geo::ray& r) {
    return ray{to_cl_float3{}(r.get_position()),
               to_cl_float3{}(r.get_direction())};
}

geo::ray convert(const ray& r) {
    return geo::ray{to_vec3{}(r.position), to_vec3{}(r.direction)};
}

triangle_verts convert(const geo::triangle_vec3& t) {
    return triangle_verts{to_cl_float3{}(std::get<0>(t.s)),
                          to_cl_float3{}(std::get<1>(t.s)),
                          to_cl_float3{}(std::get<2>(t.s))};
}

geo::triangle_vec3 convert(const triangle_verts& t) {
    return geo::triangle_vec3{
            {{to_vec3{}(t.v0), to_vec3{}(t.v1), to_vec3{}(t.v2)}}};
}

}  // namespace core
}  // namespace wayverb
