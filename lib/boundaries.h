#pragma once

#include "vec.h"
#include "cl_structs.h"

struct CuboidBoundary;

struct Boundary {
    virtual bool inside(const Vec3f & v) const = 0;
    virtual CuboidBoundary get_aabb() const = 0;
};

struct CuboidBoundary : public Boundary {
    CuboidBoundary(const Vec3f & c0 = Vec3f(), const Vec3f & c1 = Vec3f());
    bool inside(const Vec3f & v) const override;
    CuboidBoundary get_aabb() const override;
    Vec3f get_dimensions() const;
    Vec3f c0, c1;
};

CuboidBoundary get_cuboid_boundary(const std::vector<Vec3f> & vertices);

struct SphereBoundary : public Boundary {
    SphereBoundary(const Vec3f & c = Vec3f(), float radius = 0);
    bool inside(const Vec3f & v) const override;
    CuboidBoundary get_aabb() const override;

    Vec3f c;
    float radius;
    CuboidBoundary boundary;
};

using Triangle = Vec3<uint32_t>;

struct MeshBoundary : public Boundary {
    MeshBoundary(
        const std::vector<Triangle> & triangles = std::vector<Triangle>(),
        const std::vector<Vec3f> & vertices = std::vector<Vec3f>());
    bool inside(const Vec3f & v) const override;
    CuboidBoundary get_aabb() const override;

    using reference_store = std::vector<uint32_t>;

    Vec3i hash_point(const Vec3f & v) const;
    reference_store get_references(uint32_t x, uint32_t y) const;

    static const int DIVISIONS;

    std::vector<Triangle> triangles;
    std::vector<Vec3f> vertices;
    CuboidBoundary boundary;
    Vec3f cell_size;
    std::vector<std::vector<reference_store>> triangle_references;
};
