#pragma once

#include "vec.h"

#include "boundaries.h"

class Octree {
public:
    Octree(const MeshBoundary & scene_data, int max_depth);
    Octree(const MeshBoundary & scene_data, int max_depth, const CuboidBoundary & aabb, const Octree & parent);

    virtual ~Octree() noexcept = default;

    CuboidBoundary get_aabb() const;
    const std::vector<Octree> & get_nodes() const;
    const std::vector<int> & get_triangles() const;

private:
    const MeshBoundary & mesh_boundary;
    const CuboidBoundary aabb;
    std::vector<Octree> nodes;
    std::vector<int> triangles;
};
