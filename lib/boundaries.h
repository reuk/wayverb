#pragma once

#include "vec.h"
#include "cl_structs.h"

Vec3f convert(const cl_float3 & c);

struct Boundary {
    virtual bool inside(const Vec3f & v) const = 0;
};

struct CuboidBoundary : public Boundary {
    CuboidBoundary(const Vec3f & c0 = Vec3f(), const Vec3f & c1 = Vec3f());
    bool inside(const Vec3f & v) const override;
    Vec3f get_dimensions() const;
    Vec3f c0, c1;
};

CuboidBoundary get_cuboid_boundary(const std::vector<cl_float3> & vertices);

struct SphereBoundary : public Boundary {
    SphereBoundary(const Vec3f & c = Vec3f(), float radius = 0);
    bool inside(const Vec3f & v) const override;
    Vec3f c;
    float radius;
};

struct MeshBoundary : public Boundary {
    MeshBoundary(const std::vector<Triangle> & triangles = std::vector<Triangle>(),
                 const std::vector<cl_float3> & vertices = std::vector<cl_float3>());
    bool inside(const Vec3f & v) const override;

    Vec3i hash_point(const Vec3f & v) const;

    static const int DIVISIONS;

    std::vector<Triangle> triangles;
    std::vector<cl_float3> vertices;
    CuboidBoundary cuboid_boundary;
    Vec3f cell_size;

    std::vector<std::vector<std::vector<uint32_t>>> triangle_references;
};
