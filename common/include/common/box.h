#pragma once

#include "common/geometric.h"
#include "common/overlaps_2d.h"
#include "common/triangle_vec.h"

#include "glm/glm.hpp"

#include <tuple>

class copyable_scene_data;

enum class wall { nx, px, ny, py, nz, pz };
enum class direction { x, y, z };

namespace detail {
template <size_t>
struct box_data_t_trait;

template <>
struct box_data_t_trait<2> final {
    using type = glm::vec2;
};

template <>
struct box_data_t_trait<3> final {
    using type = glm::vec3;
};

}  // namespace detail

template <size_t dimensions>
class box final {
public:
    using data_t = typename detail::box_data_t_trait<dimensions>::type;

    box()
            : c0(0)
            , c1(0) {}

    box(const data_t& c0, const data_t& c1)
            : c0(glm::min(c0, c1))
            , c1(glm::max(c0, c1)) {}

    constexpr box& operator+=(const data_t& v) {
        c0 += v;
        c1 += v;
        return *this;
    }
    constexpr box& operator-=(const data_t& v) {
        c0 -= v;
        c1 -= v;
        return *this;
    }

    constexpr box& pad(float padding) {
        c0 -= padding;
        c1 += padding;
        return *this;
    }

    constexpr data_t get_c0() const { return c0; }
    constexpr data_t get_c1() const { return c1; }

    template <typename Archive>
    void serialize(Archive& archive);

private:
    data_t c0, c1;
};

template <size_t n>
using box_data_t = typename box<n>::data_t;

template <size_t dimensions>
inline auto inside(const box<dimensions>& b, const box_data_t<dimensions>& v) {
    return glm::all(glm::lessThan(b.get_c0(), v)) &&
           glm::all(glm::lessThan(v, b.get_c1()));
}

template <size_t dimensions>
inline auto centre(const box<dimensions>& b) {
    return (b.get_c0() + b.get_c1()) * 0.5f;
}

template <size_t dimensions>
inline auto dimensions(const box<dimensions>& b) {
    return b.get_c1() - b.get_c0();
}

template <size_t dimensions>
inline box<dimensions> padded(const box<dimensions>& b, float padding) {
    auto ret = b;
    return ret.pad(padding);
}

bool overlaps(const box<3>& b, const triangle_vec3& t);

template <size_t n>
inline bool overlaps(const box<2>& b, const std::array<glm::vec2, n>& shape) {
    return overlaps_2d(std::array<glm::vec2, 2>{{b.get_c0(), b.get_c1()}},
                       shape);
}

bool intersects(const box<3>& b, const geo::ray& ray, float t0, float t1);

copyable_scene_data get_scene_data(const box<3>& b);

constexpr glm::vec3 mirror_on_axis(const glm::vec3& v,
                                   const glm::vec3& pt,
                                   direction d) {
    switch (d) {
        case direction::x: return glm::vec3(2 * pt.x - v.x, v.y, v.z);
        case direction::y: return glm::vec3(v.x, 2 * pt.y - v.y, v.z);
        case direction::z: return glm::vec3(v.x, v.y, 2 * pt.z - v.z);
    }
}

constexpr glm::vec3 mirror(const box<3>& b, const glm::vec3& v, wall w) {
    switch (w) {
        case wall::nx: return mirror_on_axis(v, b.get_c0(), direction::x);
        case wall::px: return mirror_on_axis(v, b.get_c1(), direction::x);
        case wall::ny: return mirror_on_axis(v, b.get_c0(), direction::y);
        case wall::py: return mirror_on_axis(v, b.get_c1(), direction::y);
        case wall::nz: return mirror_on_axis(v, b.get_c0(), direction::z);
        case wall::pz: return mirror_on_axis(v, b.get_c1(), direction::z);
    }
}

glm::vec3 mirror_inside(const box<3>& b, const glm::vec3& v, direction d);
box<3> mirror(const box<3>& b, wall w);

template <size_t dimensions, typename It>
inline box<dimensions> min_max(It begin, It end) {
    auto mini = *begin, maxi = *begin;
    for (auto i = begin + 1; i != end; ++i) {
        mini = glm::min(*i, mini);
        maxi = glm::max(*i, maxi);
    }
    return box<dimensions>(mini, maxi);
}

//----------------------------------------------------------------------------//

template <size_t n>
constexpr bool operator==(const box<n>& a, const box<n>& b) {
    return std::make_tuple(a.get_c0(), a.get_c1()) ==
           std::make_tuple(b.get_c0(), b.get_c1());
}

template <size_t n>
constexpr bool operator!=(const box<n>& a, const box<n>& b) {
    return !(a == b);
}

template <size_t n>
inline box<n> operator+(const box<n>& a, const box_data_t<n>& b) {
    auto ret = a;
    return ret += b;
}

template <size_t n>
inline box<n> operator-(const box<n>& a, const box_data_t<n>& b) {
    auto ret = a;
    return ret -= b;
}
