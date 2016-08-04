#include "common/box.h"

#include "common/tri_cube_intersection.h"

box::box()
        : c0(0)
        , c1(0) {}

box::box(const glm::vec3& c0, const glm::vec3& c1)
        : c0(glm::min(c0, c1))
        , c1(glm::max(c0, c1)) {}

//----------------------------------------------------------------------------//

bool inside(const box& b, const glm::vec3& v) {
    return glm::all(glm::lessThan(b.get_c0(), v)) &&
           glm::all(glm::lessThan(v, b.get_c1()));
}

glm::vec3 centre(const box& b) { return (b.get_c0() + b.get_c1()) * 0.5f; }

glm::vec3 dimensions(const box& b) { return b.get_c1() - b.get_c0(); }

glm::vec3 mirror_inside(const box& b, const glm::vec3& v, direction d) {
    return mirror_on_axis(v, centre(b), d);
}

box mirror(const box& b, wall w) {
    return box(mirror(b, b.get_c0(), w), mirror(b, b.get_c1(), w));
}

bool overlaps(const box& b, const triangle_vec3& t) {
    auto coll = t;
    for (auto& i : coll) {
        i = (i - centre(b)) / dimensions(b);
    }
    return t_c_intersection(coll) == where::inside;
}

box padded(const box& b, float padding) {
    auto ret = b;
    return ret.pad(padding);
}

bool intersects(const box& b, const geo::ray& ray, float t0, float t1) {
    //  from http://people.csail.mit.edu/amy/papers/box-jgt.pdf
    auto inv = glm::vec3(1) / ray.get_direction();
    std::array<bool, 3> sign{{inv.x < 0, inv.y < 0, inv.z < 0}};
    auto get_bounds = [b](auto i) { return i ? b.get_c0() : b.get_c1(); };

    auto tmin  = (get_bounds(sign[0]).x - ray.get_position().x) * inv.x;
    auto tmax  = (get_bounds(!sign[0]).x - ray.get_position().x) * inv.x;
    auto tymin = (get_bounds(sign[1]).y - ray.get_position().y) * inv.y;
    auto tymax = (get_bounds(!sign[1]).y - ray.get_position().y) * inv.y;
    if ((tmin > tymax) || (tymin > tmax)) {
        return false;
    }
    if (tymin > tmin) {
        tmin = tymin;
    }
    if (tymax < tmax) {
        tmax = tymax;
    }
    auto tzmin = (get_bounds(sign[2]).z - ray.get_position().z) * inv.z;
    auto tzmax = (get_bounds(!sign[2]).z - ray.get_position().z) * inv.z;
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

copyable_scene_data get_scene_data(const box& b) {
    aligned::vector<cl_float3> vertices{
            {{b.get_c0().x, b.get_c0().y, b.get_c0().z}},
            {{b.get_c1().x, b.get_c0().y, b.get_c0().z}},
            {{b.get_c0().x, b.get_c1().y, b.get_c0().z}},
            {{b.get_c1().x, b.get_c1().y, b.get_c0().z}},
            {{b.get_c0().x, b.get_c0().y, b.get_c1().z}},
            {{b.get_c1().x, b.get_c0().y, b.get_c1().z}},
            {{b.get_c0().x, b.get_c1().y, b.get_c1().z}},
            {{b.get_c1().x, b.get_c1().y, b.get_c1().z}}};
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
    aligned::vector<copyable_scene_data::material> materials{
            {"default", surface{}}};

    return copyable_scene_data(triangles, vertices, materials);
}

//----------------------------------------------------------------------------//

box operator+(const box& a, const glm::vec3& b) {
    auto ret = a;
    return ret += b;
}

box operator-(const box& a, const glm::vec3& b) {
    auto ret = a;
    return ret -= b;
}
