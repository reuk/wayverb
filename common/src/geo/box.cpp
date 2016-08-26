#include "common/geo/box.h"
#include "common/geo/tri_cube_intersection.h"
#include "common/scene_data.h"

namespace geo {

glm::vec3 mirror_inside(const box& b, const glm::vec3& v, direction d) {
    return mirror_on_axis(v, centre(b), d);
}

box mirror(const box& b, wall w) {
    return box(mirror(b, b.get_min(), w), mirror(b, b.get_max(), w));
}

bool overlaps(const box& b, const triangle_vec3& t) {
    auto coll = t;
    for (auto& i : coll) {
        i = (i - centre(b)) / dimensions(b);
    }
    return t_c_intersection(coll) == where::inside;
}

bool does_intersect(const box& b, const geo::ray& ray, float t0, float t1) {
    //  from http://people.csail.mit.edu/amy/papers/box-jgt.pdf
    const auto inv = glm::vec3(1) / ray.get_direction();
    const std::array<bool, 3> sign{{inv.x < 0, inv.y < 0, inv.z < 0}};
    const auto get_bounds = [b](auto i) {
        return i ? b.get_min() : b.get_max();
    };

    auto tmin = (get_bounds(sign[0]).x - ray.get_position().x) * inv.x;
    auto tmax = (get_bounds(!sign[0]).x - ray.get_position().x) * inv.x;
    const auto tymin = (get_bounds(sign[1]).y - ray.get_position().y) * inv.y;
    const auto tymax = (get_bounds(!sign[1]).y - ray.get_position().y) * inv.y;
    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }
    if (tymin > tmin) {
        tmin = tymin;
    }
    if (tymax < tmax) {
        tmax = tymax;
    }
    const auto tzmin = (get_bounds(sign[2]).z - ray.get_position().z) * inv.z;
    const auto tzmax = (get_bounds(!sign[2]).z - ray.get_position().z) * inv.z;
    if ((tmin > tzmax) || (tzmin > tmax)) {
        return false;
    }
    if (tzmin > tmin) {
        tmin = tzmin;
    }
    if (tzmax < tmax) {
        tmax = tzmax;
    }
    return ((t0 < tmax) && (tmin < t1));
}

scene_data get_scene_data(const box& b) {
    aligned::vector<cl_float3> vertices{
            {{b.get_min().x, b.get_min().y, b.get_min().z}},
            {{b.get_max().x, b.get_min().y, b.get_min().z}},
            {{b.get_min().x, b.get_max().y, b.get_min().z}},
            {{b.get_max().x, b.get_max().y, b.get_min().z}},
            {{b.get_min().x, b.get_min().y, b.get_max().z}},
            {{b.get_max().x, b.get_min().y, b.get_max().z}},
            {{b.get_min().x, b.get_max().y, b.get_max().z}},
            {{b.get_max().x, b.get_max().y, b.get_max().z}}};
    aligned::vector<triangle> triangles{{0, 0, 1, 5},
                                        {0, 0, 4, 5},
                                        {0, 0, 1, 3},
                                        {0, 0, 2, 3},
                                        {0, 0, 2, 6},
                                        {0, 0, 4, 6},
                                        {0, 1, 5, 7},
                                        {0, 1, 3, 7},
                                        {0, 2, 3, 7},
                                        {0, 2, 6, 7},
                                        {0, 4, 5, 7},
                                        {0, 4, 6, 7}};
    aligned::vector<scene_data::material> materials{{"default", surface{}}};

    return scene_data(triangles, vertices, materials);
}

}  // namespace geo
