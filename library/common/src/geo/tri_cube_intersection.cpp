#include "common/geo/tri_cube_intersection.h"

#include "utilities/range.h"

#include "glm/glm.hpp"

namespace {
const auto eps = 10e-5;
}  // namespace

namespace geo {

int sign3(const glm::vec3& v) {
    return (v.x < eps)
                   ? 0x04
                   : 0 | (v.x > -eps)
                             ? 0x20
                             : 0 | (v.y < eps)
                                       ? 0x02
                                       : 0 | (v.y > -eps)
                                                 ? 0x10
                                                 : 0 | (v.z < eps)
                                                           ? 0x01
                                                           : 0 | (v.z > -eps)
                                                                     ? 0x08
                                                                     : 0;
}

int face_plane(const glm::vec3& p) {
    auto ret = 0;
    if (p.x > 0.5) {
        ret |= 0x01;
    }
    if (p.x < -0.5) {
        ret |= 0x02;
    }
    if (p.y > 0.5) {
        ret |= 0x04;
    }
    if (p.y < -0.5) {
        ret |= 0x08;
    }
    if (p.z > 0.5) {
        ret |= 0x10;
    }
    if (p.z < -0.5) {
        ret |= 0x20;
    }
    return ret;
}

int bevel_2d(const glm::vec3& p) {
    auto ret = 0;
    if (p.x + p.y > 1.0) {
        ret |= 0x001;
    }
    if (p.x - p.y > 1.0) {
        ret |= 0x002;
    }
    if (-p.x + p.y > 1.0) {
        ret |= 0x004;
    }
    if (-p.x - p.y > 1.0) {
        ret |= 0x008;
    }

    if (p.x + p.z > 1.0) {
        ret |= 0x010;
    }
    if (p.x - p.z > 1.0) {
        ret |= 0x020;
    }
    if (-p.x + p.z > 1.0) {
        ret |= 0x040;
    }
    if (-p.x - p.z > 1.0) {
        ret |= 0x080;
    }

    if (p.y + p.z > 1.0) {
        ret |= 0x100;
    }
    if (p.y - p.z > 1.0) {
        ret |= 0x200;
    }
    if (-p.y + p.z > 1.0) {
        ret |= 0x400;
    }
    if (-p.y - p.z > 1.0) {
        ret |= 0x800;
    }
    return ret;
}

int bevel_3d(const glm::vec3& p) {
    auto ret = 0;
    if (p.x + p.y + p.z > 1.5) {
        ret |= 0x01;
    }
    if (p.x + p.y - p.z > 1.5) {
        ret |= 0x02;
    }
    if (p.x - p.y + p.z > 1.5) {
        ret |= 0x04;
    }
    if (p.x - p.y - p.z > 1.5) {
        ret |= 0x08;
    }
    if (-p.x + p.y + p.z > 1.5) {
        ret |= 0x10;
    }
    if (-p.x + p.y - p.z > 1.5) {
        ret |= 0x20;
    }
    if (-p.x - p.y + p.z > 1.5) {
        ret |= 0x40;
    }
    if (-p.x - p.y - p.z > 1.5) {
        ret |= 0x80;
    }
    return ret;
}

int check_point(const glm::vec3& p1,
                const glm::vec3& p2,
                float alpha,
                int mask) {
    return face_plane(glm::mix(p1, p2, alpha)) & mask;
}

where check_line(const glm::vec3& p1, const glm::vec3& p2, int outcode_diff) {
    if (0x01 & outcode_diff) {
        if (!check_point(p1, p2, (0.5 - p1.x) / (p2.x - p1.x), 0x3e)) {
            return where::inside;
        }
    }
    if (0x02 & outcode_diff) {
        if (!check_point(p1, p2, (-0.5 - p1.x) / (p2.x - p1.x), 0x3d)) {
            return where::inside;
        }
    }
    if (0x04 & outcode_diff) {
        if (!check_point(p1, p2, (0.5 - p1.y) / (p2.y - p1.y), 0x3b)) {
            return where::inside;
        }
    }
    if (0x08 & outcode_diff) {
        if (!check_point(p1, p2, (-0.5 - p1.y) / (p2.y - p1.y), 0x37)) {
            return where::inside;
        }
    }
    if (0x10 & outcode_diff) {
        if (!check_point(p1, p2, (0.5 - p1.z) / (p2.z - p1.z), 0x2f)) {
            return where::inside;
        }
    }
    if (0x20 & outcode_diff) {
        if (!check_point(p1, p2, (-0.5 - p1.z) / (p2.z - p1.z), 0x1f)) {
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
    const auto mm = enclosing_range(std::begin(coll), std::end(coll));

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

    const auto mm = enclosing_range(std::begin(v), std::end(v));

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
