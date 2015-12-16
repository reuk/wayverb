#pragma once

#include "vec.h"

#include "scene_data.h"
#include "boundaries.h"

class Octree {
public:
    Octree(const SceneData & mesh_boundary,
           int max_depth,
           const std::vector<int> to_test,
           const CuboidBoundary & aabb);
    Octree(const SceneData & mesh_boundary, int max_depth);

    virtual ~Octree() noexcept = default;

    CuboidBoundary get_aabb() const;
    const std::vector<Octree> & get_nodes() const;
    const std::vector<int> & get_triangles() const;

private:
    const SceneData & scene_data;
    const CuboidBoundary aabb;
    std::vector<Octree> nodes;
    std::vector<int> triangles;
};

std::vector<int> get_static_octree(const Octree & o);
