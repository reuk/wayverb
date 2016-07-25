#pragma once

#include "reduce.h"
#include "scene_data.h"
#include "triangle.h"
#include "triangle_vec.h"

#include "common/aligned/vector.h"

#include "glm/glm.hpp"

namespace geo {
class Ray;
}  // namespace geo

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

    Box() = default;

    Box(const glm::vec3& c0, const glm::vec3& c1)
            : c0(glm::min(c0, c1))
            , c1(glm::max(c0, c1)) {}

    bool inside(const glm::vec3& v) const {
        return glm::all(glm::lessThan(c0, v)) && glm::all(glm::lessThan(v, c1));
    }

    glm::vec3 centre() const { return glm::vec3((c0 + c1) * 0.5f); }

    glm::vec3 mirror_on_axis(const glm::vec3& v,
                             const glm::vec3& pt,
                             Direction d) const {
        switch (d) {
            case Direction::x: return glm::vec3(2 * pt.x - v.x, v.y, v.z);
            case Direction::y: return glm::vec3(v.x, 2 * pt.y - v.y, v.z);
            case Direction::z: return glm::vec3(v.x, v.y, 2 * pt.z - v.z);
        }
    }

    glm::vec3 mirror_inside(const glm::vec3& v, Direction d) const {
        return mirror_on_axis(v, centre(), d);
    }

    glm::vec3 mirror(const glm::vec3& v, Wall w) const {
        switch (w) {
            case Wall::nx: return mirror_on_axis(v, c0, Direction::x);
            case Wall::px: return mirror_on_axis(v, c1, Direction::x);
            case Wall::ny: return mirror_on_axis(v, c0, Direction::y);
            case Wall::py: return mirror_on_axis(v, c1, Direction::y);
            case Wall::nz: return mirror_on_axis(v, c0, Direction::z);
            case Wall::pz: return mirror_on_axis(v, c1, Direction::z);
        }
    }

    Box mirror(Wall w) const { return Box(mirror(c0, w), mirror(c1, w)); }

    bool operator==(const Box& b) const {
        return glm::all(glm::equal(c0, b.c0)) && glm::all(glm::equal(c1, b.c1));
    }

    Box operator+(const glm::vec3& v) const { return Box(c0 + v, c1 + v); }

    Box operator-(const glm::vec3& v) const { return Box(c0 - v, c1 - v); }

    glm::vec3 dimensions() const { return c1 - c0; }

    glm::vec3 get_c0() const { return c0; }
    glm::vec3 get_c1() const { return c1; }

    template <typename Archive>
    friend void serialize(Archive& archive, Box& m);

private:
    glm::vec3 c0, c1;
};

class CuboidBoundary;

class Boundary {
public:
    Boundary()                    = default;
    virtual ~Boundary() noexcept  = default;
    Boundary(Boundary&&) noexcept = default;
    Boundary& operator=(Boundary&&) noexcept = default;
    Boundary(const Boundary&)                = default;
    Boundary& operator=(const Boundary&)          = default;
    virtual bool inside(const glm::vec3& v) const = 0;
    virtual CuboidBoundary get_aabb() const       = 0;

    template <typename Archive>
    friend void serialize(Archive& archive, Boundary& m);
};

template <typename T>
inline Box min_max(const T& vertices) {
    glm::vec3 mini, maxi;
    mini = maxi = vertices.front();
    for (auto i = vertices.begin() + 1; i != vertices.end(); ++i) {
        mini = glm::min(*i, mini);
        maxi = glm::max(*i, maxi);
    }
    return Box(mini, maxi);
}

class CuboidBoundary : public Boundary, public Box {
public:
    explicit CuboidBoundary(const Box& b = Box());
    CuboidBoundary(const glm::vec3& c0, const glm::vec3& c1);
    bool inside(const glm::vec3& v) const override;
    bool overlaps(const TriangleVec3& t) const;
    CuboidBoundary get_aabb() const override;
    CuboidBoundary get_padded(float padding) const;
    bool intersects(const geo::Ray& ray, float t0, float t1);

    CopyableSceneData get_scene_data() const;

    template <typename Archive>
    friend void serialize(Archive& archive, CuboidBoundary& m);
};

class SphereBoundary : public Boundary {
public:
    explicit SphereBoundary(const glm::vec3& c = glm::vec3(),
                            float radius       = 0,
                            const aligned::vector<Surface>& surfaces =
                                    aligned::vector<Surface>{Surface{}});
    bool inside(const glm::vec3& v) const override;
    CuboidBoundary get_aabb() const override;

private:
    glm::vec3 c;
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
    MeshBoundary(const aligned::vector<Triangle>& triangles,
                 const aligned::vector<glm::vec3>& vertices,
                 const aligned::vector<Surface>& surfaces);
    explicit MeshBoundary(const CopyableSceneData& sd);
    bool inside(const glm::vec3& v) const override;
    CuboidBoundary get_aabb() const override;

    aligned::vector<size_t> get_triangle_indices() const;

    using reference_store = aligned::vector<uint32_t>;

    glm::ivec3 hash_point(const glm::vec3& v) const;
    const reference_store& get_references(int x, int y) const;
    const reference_store& get_references(const glm::ivec3& i) const;

    static constexpr int DIVISIONS = 1024;

    const aligned::vector<Triangle>& get_triangles() const;
    const aligned::vector<glm::vec3>& get_vertices() const;
    const CuboidBoundary& get_boundary() const;
    const aligned::vector<Surface>& get_surfaces() const;
    glm::vec3 get_cell_size() const;

private:
    using hash_table = aligned::vector<aligned::vector<reference_store>>;

    hash_table compute_triangle_references() const;

    aligned::vector<Triangle> triangles;
    aligned::vector<glm::vec3> vertices;
    aligned::vector<Surface> surfaces;
    CuboidBoundary boundary;
    glm::vec3 cell_size;
    hash_table triangle_references;

    static const reference_store empty_reference_store;
};
