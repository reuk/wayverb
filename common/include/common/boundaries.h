#pragma once

#include "reduce.h"
#include "scene_data.h"
#include "triangle.h"
#include "triangle_vec.h"
#include "vec.h"

namespace geo {
class Ray;
}  // namespace geo

template <typename F>
Vec3f sub_elementwise(const std::vector<Vec3f>& coll, const F& f = F()) {
    return std::accumulate(
        coll.begin() + 1, coll.end(), coll.front(), vec::ApplyFunctor<F>(f));
}

template <typename F, typename T>
constexpr Vec3f sub_elementwise(const T& coll, const F& f = F()) {
    return reduce(coll, vec::ApplyFunctor<F>(f));
}

template <typename T>
struct min_functor {
    constexpr T operator()(T a, T b) const {
        return std::min(a, b);
    }
};

template <typename T>
struct max_functor {
    constexpr T operator()(T a, T b) const {
        return std::max(a, b);
    }
};

template <typename T>
constexpr Vec3f get_max(const T& coll) {
    return sub_elementwise<max_functor<float>>(coll);
}

template <typename T>
constexpr Vec3f get_min(const T& coll) {
    return sub_elementwise<min_functor<float>>(coll);
}

struct Box {
    enum class Wall {
        nx,
        px,
        ny,
        py,
        nz,
        pz,
    };

    enum class Direction {
        x,
        y,
        z,
    };

    constexpr Box() = default;

    constexpr Box(const Vec3f& c0, const Vec3f& c1)
            : Box(std::array<Vec3f, 2>{{c0, c1}}) {
    }

    constexpr explicit Box(const std::array<Vec3f, 2>& v)
            : c0(get_min(v))
            , c1(get_max(v)) {
    }

    constexpr bool inside(const Vec3f& v) const {
        return (c0 < v).all() && (v < c1).all();
    }

    constexpr Vec3f centre() const {
        return Vec3f((c0 + c1) * 0.5);
    }

    constexpr Vec3f mirror_on_axis(const Vec3f& v,
                                   const Vec3f& pt,
                                   Direction d) const {
        switch (d) {
            case Direction::x:
                return Vec3f(2 * pt.x - v.x, v.y, v.z);
            case Direction::y:
                return Vec3f(v.x, 2 * pt.y - v.y, v.z);
            case Direction::z:
                return Vec3f(v.x, v.y, 2 * pt.z - v.z);
        }
    }

    constexpr Vec3f mirror_inside(const Vec3f& v, Direction d) const {
        return mirror_on_axis(v, centre(), d);
    }

    constexpr Vec3f mirror(const Vec3f& v, Wall w) const {
        switch (w) {
            case Wall::nx:
                return mirror_on_axis(v, c0, Direction::x);
            case Wall::px:
                return mirror_on_axis(v, c1, Direction::x);
            case Wall::ny:
                return mirror_on_axis(v, c0, Direction::y);
            case Wall::py:
                return mirror_on_axis(v, c1, Direction::y);
            case Wall::nz:
                return mirror_on_axis(v, c0, Direction::z);
            case Wall::pz:
                return mirror_on_axis(v, c1, Direction::z);
        }
    }

    constexpr Box mirror(Wall w) const {
        return Box(mirror(c0, w), mirror(c1, w));
    }

    constexpr bool operator==(const Box& b) const {
        return (c0 == b.c0).all() && (c1 == b.c1).all();
    }

    constexpr Box operator+(const Vec3f& v) const {
        return Box(c0 + v, c1 + v);
    }

    constexpr Box operator-(const Vec3f& v) const {
        return Box(c0 - v, c1 - v);
    }

    constexpr Vec3f dimensions() const {
        return c1 - c0;
    }

    constexpr Vec3f get_c0() const {
        return c0;
    }
    constexpr Vec3f get_c1() const {
        return c1;
    }

    template <typename Archive>
    friend void serialize(Archive& archive, Box& m);

private:
    Vec3f c0, c1;
};

class CuboidBoundary;

class Boundary {
public:
    Boundary() = default;
    virtual ~Boundary() noexcept = default;
    Boundary(Boundary&&) noexcept = default;
    Boundary& operator=(Boundary&&) noexcept = default;
    Boundary(const Boundary&) = default;
    Boundary& operator=(const Boundary&) = default;
    virtual bool inside(const Vec3f& v) const = 0;
    virtual CuboidBoundary get_aabb() const = 0;

    template <typename Archive>
    friend void serialize(Archive& archive, Boundary& m);
};

Box get_surrounding_box(const std::vector<Vec3f>& vertices);

class CuboidBoundary : public Boundary, public Box {
public:
    explicit CuboidBoundary(const Box& b = Box());
    CuboidBoundary(const Vec3f& c0, const Vec3f& c1);
    bool inside(const Vec3f& v) const override;
    bool overlaps(const TriangleVec3f& t) const;
    CuboidBoundary get_aabb() const override;
    CuboidBoundary get_padded(float padding) const;
    bool intersects(const geo::Ray& ray, float t0, float t1);

    CopyableSceneData get_scene_data() const;

    template <typename Archive>
    friend void serialize(Archive& archive, CuboidBoundary& m);
};

class SphereBoundary : public Boundary {
public:
    explicit SphereBoundary(
        const Vec3f& c = Vec3f(),
        float radius = 0,
        const std::vector<Surface>& surfaces = std::vector<Surface>{Surface{}});
    bool inside(const Vec3f& v) const override;
    CuboidBoundary get_aabb() const override;

private:
    Vec3f c;
    float radius;
    CuboidBoundary boundary;
};

template <typename T>
inline bool almost_equal(T x, T y, int ups) {
    return std::abs(x - y) <= std::numeric_limits<T>::epsilon() *
                                  std::max(std::abs(x), std::abs(y)) * ups;
}

class MeshBoundary : public Boundary {
public:
    MeshBoundary(const std::vector<Triangle>& triangles,
                 const std::vector<Vec3f>& vertices,
                 const std::vector<Surface>& surfaces);
    explicit MeshBoundary(const CopyableSceneData& sd);
    bool inside(const Vec3f& v) const override;
    CuboidBoundary get_aabb() const override;

    std::vector<int> get_triangle_indices() const;

    using reference_store = std::vector<uint32_t>;

    Vec3i hash_point(const Vec3f& v) const;
    const reference_store& get_references(int x, int y) const;
    const reference_store& get_references(const Vec3i& i) const;

    static constexpr int DIVISIONS = 1024;

    const std::vector<Triangle>& get_triangles() const;
    const std::vector<Vec3f>& get_vertices() const;
    const CuboidBoundary& get_boundary() const;
    const std::vector<Surface>& get_surfaces() const;
    Vec3f get_cell_size() const;

private:
    using hash_table = std::vector<std::vector<reference_store>>;

    hash_table compute_triangle_references() const;

    std::vector<Triangle> triangles;
    std::vector<Vec3f> vertices;
    std::vector<Surface> surfaces;
    CuboidBoundary boundary;
    Vec3f cell_size;
    hash_table triangle_references;

    static const reference_store empty_reference_store;
};
