#pragma once

#include "common/geometric.h"
#include "common/scene_data.h"
#include "common/triangle_vec.h"
#include "glm/glm.hpp"
#include <tuple>

enum class wall { nx, px, ny, py, nz, pz };
enum class direction { x, y, z };

class box final {
public:
    box();
    box(const glm::vec3& c0, const glm::vec3& c1);

    constexpr box& operator+=(const glm::vec3& v) {
        c0 += v;
        c1 += v;
        return *this;
    }
    constexpr box& operator-=(const glm::vec3& v) {
        c0 -= v;
        c1 -= v;
        return *this;
    }

    constexpr box& pad(float padding) {
        c0 -= padding;
        c1 += padding;
        return *this;
    }

    constexpr glm::vec3 get_c0() const { return c0; }
    constexpr glm::vec3 get_c1() const { return c1; }

    template <typename Archive>
    void serialize(Archive& archive);

private:
    glm::vec3 c0, c1;
};

bool inside(const box& b, const glm::vec3& v);

glm::vec3 centre(const box& b);

glm::vec3 dimensions(const box& b);

bool overlaps(const box& b, const TriangleVec3& t);

box padded(const box& b, float padding);

bool intersects(const box& b, const geo::Ray& ray, float t0, float t1);

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
        case wall::nx: return mirror_on_axis(v, b.get_c0(), direction::x);
        case wall::px: return mirror_on_axis(v, b.get_c1(), direction::x);
        case wall::ny: return mirror_on_axis(v, b.get_c0(), direction::y);
        case wall::py: return mirror_on_axis(v, b.get_c1(), direction::y);
        case wall::nz: return mirror_on_axis(v, b.get_c0(), direction::z);
        case wall::pz: return mirror_on_axis(v, b.get_c1(), direction::z);
    }
}

glm::vec3 mirror_inside(const box& b, const glm::vec3& v, direction d);

box mirror(const box& b, wall w);

template <typename It>
inline box min_max(It begin, It end) {
    auto mini = *begin, maxi = *begin;
    for (auto i = begin + 1; i != end; ++i) {
        mini = glm::min(*i, mini);
        maxi = glm::max(*i, maxi);
    }
    return box(mini, maxi);
}

//----------------------------------------------------------------------------//

constexpr bool operator==(const box& a, const box& b) {
    return std::make_tuple(a.get_c0(), a.get_c1()) ==
           std::make_tuple(b.get_c0(), b.get_c1());
}

constexpr bool operator!=(const box& a, const box& b) { return !(a == b); }

box operator+(const box& a, const glm::vec3& b);
box operator-(const box& a, const glm::vec3& b);
