#include "core/geo/tri_cube_intersection.h"

#include "utilities/range.h"

#include "glm/glm.hpp"

namespace wayverb {
namespace core {

namespace {
const auto eps = 10e-5;
}  // namespace

namespace geo {

int sign3(const glm::vec3& v) {
    return ((v.x < eps) ? (1 << 2) : 0) | ((-eps < v.x) ? (1 << 5) : 0) |
           ((v.y < eps) ? (1 << 1) : 0) | ((-eps < v.y) ? (1 << 4) : 0) |
           ((v.z < eps) ? (1 << 0) : 0) | ((-eps < v.z) ? (1 << 3) : 0);
}

int face_plane(const glm::vec3& p) {
    return ((0.5 < p.x) ? (1 << 0) : 0) | ((p.x < -0.5) ? (1 << 1) : 0) |
           ((0.5 < p.y) ? (1 << 2) : 0) | ((p.y < -0.5) ? (1 << 3) : 0) |
           ((0.5 < p.z) ? (1 << 4) : 0) | ((p.z < -0.5) ? (1 << 5) : 0);
}

int bevel_2d(const glm::vec3& p) {
    return ((1.0 < p.x + p.y) ? (1 << 0) : 0) |
           ((1.0 < p.x - p.y) ? (1 << 1) : 0) |
           ((1.0 < -p.x + p.y) ? (1 << 2) : 0) |
           ((1.0 < -p.x - p.y) ? (1 << 3) : 0) |
           ((1.0 < p.x + p.z) ? (1 << 4) : 0) |
           ((1.0 < p.x - p.z) ? (1 << 5) : 0) |
           ((1.0 < -p.x + p.z) ? (1 << 6) : 0) |
           ((1.0 < -p.x - p.z) ? (1 << 7) : 0) |
           ((1.0 < p.y + p.z) ? (1 << 8) : 0) |
           ((1.0 < p.y - p.z) ? (1 << 9) : 0) |
           ((1.0 < -p.y + p.z) ? (1 << 10) : 0) |
           ((1.0 < -p.y - p.z) ? (1 << 11) : 0);
}

int bevel_3d(const glm::vec3& p) {
    return ((1.5 < p.x + p.y + p.z) ? (1 << 0) : 0) |
           ((1.5 < p.x + p.y - p.z) ? (1 << 1) : 0) |
           ((1.5 < p.x - p.y + p.z) ? (1 << 2) : 0) |
           ((1.5 < p.x - p.y - p.z) ? (1 << 3) : 0) |
           ((1.5 < -p.x + p.y + p.z) ? (1 << 4) : 0) |
           ((1.5 < -p.x + p.y - p.z) ? (1 << 5) : 0) |
           ((1.5 < -p.x - p.y + p.z) ? (1 << 6) : 0) |
           ((1.5 < -p.x - p.y - p.z) ? (1 << 7) : 0);
}

where check_point(const glm::vec3& p1,
                  const glm::vec3& p2,
                  float alpha,
                  int mask) {
    return ((face_plane(glm::mix(p1, p2, alpha)) & mask) == 0) ? where::inside
                                                               : where::outside;
}

where check_line(const glm::vec3& p1, const glm::vec3& p2, int outcode_diff) {
    if (((1 << 0) & outcode_diff) != 0) {
        if (check_point(p1, p2, (0.5 - p1.x) / (p2.x - p1.x), 0x3e) ==
            where::inside) {
            return where::inside;
        }
    }
    if (((1 << 1) & outcode_diff) != 0) {
        if (check_point(p1, p2, (-0.5 - p1.x) / (p2.x - p1.x), 0x3d) ==
            where::inside) {
            return where::inside;
        }
    }
    if (((1 << 2) & outcode_diff) != 0) {
        if (check_point(p1, p2, (0.5 - p1.y) / (p2.y - p1.y), 0x3b) ==
            where::inside) {
            return where::inside;
        }
    }
    if (((1 << 3) & outcode_diff) != 0) {
        if (check_point(p1, p2, (-0.5 - p1.y) / (p2.y - p1.y), 0x37) ==
            where::inside) {
            return where::inside;
        }
    }
    if (((1 << 4) & outcode_diff) != 0) {
        if (check_point(p1, p2, (0.5 - p1.z) / (p2.z - p1.z), 0x2f) ==
            where::inside) {
            return where::inside;
        }
    }
    if (((1 << 5) & outcode_diff) != 0) {
        if (check_point(p1, p2, (-0.5 - p1.z) / (p2.z - p1.z), 0x1f) ==
            where::inside) {
            return where::inside;
        }
    }
    return where::outside;
}

where point_triangle_intersection(const glm::vec3& p, const triangle_vec3& t) {
    auto v0 = t[0];
    auto v1 = t[1];
    auto v2 = t[2];

    const std::array<glm::vec3, 3> coll{{v0, v1, v2}};
    const auto mm = util::enclosing_range(std::begin(coll), std::end(coll));

    if (glm::any(glm::lessThan(mm.get_max(), p))) {
        return where::outside;
    }
    if (glm::any(glm::lessThan(p, mm.get_min()))) {
        return where::outside;
    }

    auto get_sign = [&p](auto a, auto b) {
        auto vec_a = a - b;
        auto vec_p = a - p;
        auto cross = glm::cross(vec_a, vec_p);
        return sign3(cross);
    };

    auto sign12 = get_sign(v0, v1);
    auto sign23 = get_sign(v1, v2);
    auto sign31 = get_sign(v2, v0);

    return ((sign12 & sign23 & sign31) == 0) ? where::outside : where::inside;
}

//  from
//  http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox_tam.pdf
where t_c_intersection(const triangle_vec3& t) {
    std::array<glm::vec3, 3> v{{t[0], t[1], t[2]}};
    std::array<glm::vec3, 3> f{{v[1] - v[0], v[2] - v[1], v[0] - v[2]}};

    for (const auto& a : {glm::vec3(0, -f[0].z, f[0].y),
                          glm::vec3(0, -f[1].z, f[1].y),
                          glm::vec3(0, -f[2].z, f[2].y),
                          glm::vec3(f[0].z, 0, -f[0].x),
                          glm::vec3(f[1].z, 0, -f[1].x),
                          glm::vec3(f[2].z, 0, -f[2].x),
                          glm::vec3(-f[0].y, f[0].x, 0),
                          glm::vec3(-f[1].y, f[1].x, 0),
                          glm::vec3(-f[2].y, f[2].x, 0)}) {
        auto coll = {glm::dot(a, v[0]), glm::dot(a, v[1]), glm::dot(a, v[2])};
        auto r = glm::dot(glm::abs(a), glm::vec3(0.5));
        if (std::max(-std::max(coll), std::min(coll)) > r) {
            return where::outside;
        }
    }

    const auto mm = util::enclosing_range(std::begin(v), std::end(v));

    if (glm::any(glm::lessThan(mm.get_max(), glm::vec3(-0.5)))) {
        return where::outside;
    }
    if (glm::any(glm::lessThan(glm::vec3(0.5), mm.get_min()))) {
        return where::outside;
    }

    auto normal = glm::normalize(glm::cross(f[0], f[2]));
    auto dist = glm::dot(normal, v[0]);

    auto r = glm::dot(glm::abs(normal), glm::vec3(0.5));
    return fabs(dist) <= r ? where::inside : where::outside;
}

}  // namespace geo
}  // namespace core
}  // namespace wayverb
