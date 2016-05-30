#include "common/tri_cube_intersection.h"

#include "common/boundaries.h"

static const auto eps = 10e-5;

int sign3(const glm::vec3& v) {
    return (v.x < eps) ? 0x04
                       : 0 | (v.x > -eps)
                             ? 0x20
                             : 0 | (v.y < eps)
                                   ? 0x02
                                   : 0 | (v.y > -eps)
                                         ? 0x10
                                         : 0 | (v.z < eps)
                                               ? 0x01
                                               : 0 | (v.z > -eps) ? 0x08 : 0;
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

Rel check_line(const glm::vec3& p1, const glm::vec3& p2, int outcode_diff) {
    if (0x01 & outcode_diff) {
        if (!check_point(p1, p2, (0.5 - p1.x) / (p2.x - p1.x), 0x3e)) {
            return Rel::idInside;
        }
    }
    if (0x02 & outcode_diff) {
        if (!check_point(p1, p2, (-0.5 - p1.x) / (p2.x - p1.x), 0x3d)) {
            return Rel::idInside;
        }
    }
    if (0x04 & outcode_diff) {
        if (!check_point(p1, p2, (0.5 - p1.y) / (p2.y - p1.y), 0x3b)) {
            return Rel::idInside;
        }
    }
    if (0x08 & outcode_diff) {
        if (!check_point(p1, p2, (-0.5 - p1.y) / (p2.y - p1.y), 0x37)) {
            return Rel::idInside;
        }
    }
    if (0x10 & outcode_diff) {
        if (!check_point(p1, p2, (0.5 - p1.z) / (p2.z - p1.z), 0x2f)) {
            return Rel::idInside;
        }
    }
    if (0x20 & outcode_diff) {
        if (!check_point(p1, p2, (-0.5 - p1.z) / (p2.z - p1.z), 0x1f)) {
            return Rel::idInside;
        }
    }
    return Rel::idOutside;
}

Rel point_triangle_intersection(const glm::vec3& p, const TriangleVec3& t) {
    auto v0 = t[0];
    auto v1 = t[1];
    auto v2 = t[2];

    const auto mm = min_max(std::array<glm::vec3, 3>{{v0, v1, v2}});

    if (glm::any(glm::lessThan(mm.get_c1(), p))) {
        return Rel::idOutside;
    }
    if (glm::any(glm::lessThan(p, mm.get_c0()))) {
        return Rel::idOutside;
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

    return ((sign12 & sign23 & sign31) == 0) ? Rel::idOutside : Rel::idInside;
}

/*
Rel t_c_intersection(const TriangleVerts & t) {
    auto v0 = t[0];
    auto v1 = t[1];
    auto v2 = t[2];

    auto v0_test = face_plane(v0);
    if (v0_test == 0)
        return Rel::idInside;
    auto v1_test = face_plane(v1);
    if (v1_test == 0)
        return Rel::idInside;
    auto v2_test = face_plane(v2);
    if (v2_test == 0)
        return Rel::idInside;
    if ((v0_test & v1_test & v2_test) != 0)
        return Rel::idOutside;

    v0_test |= bevel_2d(v0) << 8;
    v1_test |= bevel_2d(v1) << 8;
    v2_test |= bevel_2d(v2) << 8;
    if ((v0_test & v1_test & v2_test) != 0)
        return Rel::idOutside;

    v0_test |= bevel_3d(v0) << 24;
    v1_test |= bevel_3d(v1) << 24;
    v2_test |= bevel_3d(v2) << 24;
    if ((v0_test & v1_test & v2_test) != 0)
        return Rel::idOutside;

    auto edge_test = [](auto a, auto b, auto a_test, auto b_test) {
        if ((a_test & b_test) == 0) {
            if (check_line(a, b, a_test | b_test) == Rel::idInside) {
                return Rel::idInside;
            }
        }
        return Rel::idOutside;
    };

    if (edge_test(v0, v1, v0_test, v1_test) == Rel::idInside)
        return Rel::idInside;
    if (edge_test(v1, v2, v1_test, v2_test) == Rel::idInside)
        return Rel::idInside;
    if (edge_test(v0, v2, v0_test, v2_test) == Rel::idInside)
        return Rel::idInside;

    auto vect01 = v0 - v1;
    auto vect02 = v0 - v2;
    auto norm = vect01.cross(vect02);

    auto d = norm.dot(v0);

    auto normal_check = [&d, &t](auto norm, auto x, auto y, auto z) {
        Vec3f mul(x, y, z);
        auto denom = norm.dot(mul);
        if (fabs(denom) > eps) {
            auto pos = d / denom;
            auto hit = mul * pos;
            if (fabs(hit.x) <= 0.5) {
                if (point_triangle_intersection(hit, t) == Rel::idInside) {
                    return Rel::idInside;
                }
            }
        }
        return Rel::idOutside;
    };

    if (normal_check(norm, 1, 1, 1) == Rel::idInside)
        return Rel::idInside;
    if (normal_check(norm, 1, 1, -1) == Rel::idInside)
        return Rel::idInside;
    if (normal_check(norm, 1, -1, 1) == Rel::idInside)
        return Rel::idInside;
    if (normal_check(norm, 1, -1, -1) == Rel::idInside)
        return Rel::idInside;

    return Rel::idOutside;
}
 */

//  from
//  http://fileadmin.cs.lth.se/cs/Personal/Tomas_Akenine-Moller/code/tribox_tam.pdf
Rel t_c_intersection(const TriangleVec3& t) {
    std::array<glm::vec3, 3> v{{
        t[0], t[1], t[2],
    }};
    std::array<glm::vec3, 3> f{{v[1] - v[0], v[2] - v[1], v[0] - v[2]}};

    for (const auto& a : {
             glm::vec3(0, -f[0].z, f[0].y),
             glm::vec3(0, -f[1].z, f[1].y),
             glm::vec3(0, -f[2].z, f[2].y),
             glm::vec3(f[0].z, 0, -f[0].x),
             glm::vec3(f[1].z, 0, -f[1].x),
             glm::vec3(f[2].z, 0, -f[2].x),
             glm::vec3(-f[0].y, f[0].x, 0),
             glm::vec3(-f[1].y, f[1].x, 0),
             glm::vec3(-f[2].y, f[2].x, 0),
         }) {
        auto coll = {glm::dot(a, v[0]), glm::dot(a, v[1]), glm::dot(a, v[2])};
        auto r = glm::dot(glm::abs(a), glm::vec3(0.5));
        if (std::max(-std::max(coll), std::min(coll)) > r) {
            return Rel::idOutside;
        }
    }

    const auto mm = min_max(v);

    if (glm::any(glm::lessThan(mm.get_c1(), glm::vec3(-0.5)))) {
        return Rel::idOutside;
    }
    if (glm::any(glm::lessThan(glm::vec3(0.5), mm.get_c0()))) {
        return Rel::idOutside;
    }

    auto normal = glm::normalize(glm::cross(f[0], f[2]));
    auto dist = glm::dot(normal, v[0]);

    auto r = glm::dot(glm::abs(normal), glm::vec3(0.5));
    return fabs(dist) <= r ? Rel::idInside : Rel::idOutside;
}
