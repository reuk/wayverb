#pragma once

#include "scene_data.h"
#include "cl_structs.h"

struct CuboidBoundary;

struct Boundary {
    virtual ~Boundary() noexcept = default;
    virtual bool inside(const Vec3f& v) const = 0;
    virtual CuboidBoundary get_aabb() const = 0;
};

struct CuboidBoundary : public Boundary {
    CuboidBoundary(const Vec3f& c0 = Vec3f(), const Vec3f& c1 = Vec3f());
    virtual ~CuboidBoundary() noexcept = default;
    bool inside(const Vec3f& v) const override;
    CuboidBoundary get_aabb() const override;
    Vec3f get_dimensions() const;
    const Vec3f c0, c1;
};

CuboidBoundary get_cuboid_boundary(const std::vector<Vec3f>& vertices);

struct SphereBoundary : public Boundary {
    SphereBoundary(const Vec3f& c = Vec3f(), float radius = 0);
    virtual ~SphereBoundary() noexcept = default;
    bool inside(const Vec3f& v) const override;
    CuboidBoundary get_aabb() const override;

    const Vec3f c;
    const float radius;
    const CuboidBoundary boundary;
};

struct MeshBoundary : public Boundary {
    MeshBoundary(
        const std::vector<Triangle>& triangles = std::vector<Triangle>(),
        const std::vector<Vec3f>& vertices = std::vector<Vec3f>());
    MeshBoundary(const SceneData& sd);
    virtual ~MeshBoundary() noexcept = default;
    bool inside(const Vec3f& v) const override;
    CuboidBoundary get_aabb() const override;

    using reference_store = std::vector<uint32_t>;

    Vec3i hash_point(const Vec3f& v) const;
    reference_store get_references(int x, int y) const;
    reference_store get_references(const Vec3i& i) const;

    const int DIVISIONS{1024};

    const std::vector<Triangle> triangles;
    const std::vector<Vec3f> vertices;
    const CuboidBoundary boundary;
    const Vec3f cell_size;
    const std::vector<std::vector<reference_store>> triangle_references;

private:
    MeshBoundary(
        const std::tuple<std::vector<Triangle>, std::vector<Vec3f>>& data);
    std::vector<std::vector<reference_store>> get_triangle_references() const;
};
