#pragma once

#include "common/geo/geometric.h"
#include "common/geo/triangle_vec.h"
#include "common/range.h"

#include "glm/glm.hpp"

class copyable_scene_data;

namespace geo {

using box = util::range<glm::vec3>;

enum class wall { nx, px, ny, py, nz, pz };
enum class direction { x, y, z };

bool overlaps(const box& b, const triangle_vec3& t);

bool does_intersect(const box& b, const geo::ray& ray, float t0, float t1);

copyable_scene_data get_scene_data(const box& b);

constexpr glm::vec3 mirror_on_axis(const glm::vec3& v,
                                   const glm::vec3& pt,
                                   direction d) {
    switch (d) {
        case direction::x: return glm::vec3(2 * pt.x - v.x, v.y, v.z);
        case direction::y: return glm::vec3(v.x, 2 * pt.y - v.y, v.z);
        case direction::z: return glm::vec3(v.x, v.y, 2 * pt.z - v.z);
    }
}

constexpr glm::vec3 mirror(const box& b, const glm::vec3& v, wall w) {
    switch (w) {
        case wall::nx: return mirror_on_axis(v, b.get_min(), direction::x);
        case wall::px: return mirror_on_axis(v, b.get_max(), direction::x);
        case wall::ny: return mirror_on_axis(v, b.get_min(), direction::y);
        case wall::py: return mirror_on_axis(v, b.get_max(), direction::y);
        case wall::nz: return mirror_on_axis(v, b.get_min(), direction::z);
        case wall::pz: return mirror_on_axis(v, b.get_max(), direction::z);
    }
}

glm::vec3 mirror_inside(const box& b, const glm::vec3& v, direction d);
box mirror(const box& b, wall w);

}  // namespace geo
