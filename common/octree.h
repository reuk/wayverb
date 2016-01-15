#pragma once

#include "scene_data.h"
#include "boundaries.h"
#include "geometric.h"

typedef union {
    cl_float f;
    cl_uint i;
} __attribute__((aligned(8))) OctreeInfoField;

class Octree final {
public:
    Octree() = default;
    Octree(const SceneData& mesh_boundary, int max_depth, float padding = 0);
    Octree(const SceneData& mesh_boundary,
           int max_depth,
           const std::vector<int>& to_test,
           const CuboidBoundary& aabb);

    CuboidBoundary get_aabb() const;
    bool has_nodes() const;
    const std::array<Octree, 8>& get_nodes() const;
    const std::vector<int>& get_triangles() const;
    std::vector<OctreeInfoField> get_flattened() const;

    int get_side() const;

    std::vector<const Octree*> intersect(const geo::Ray& ray) const;
    const Octree& get_surrounding_leaf(const Vec3f& v) const;

private:
    void fill_flattened(std::vector<OctreeInfoField>& ret) const;

    CuboidBoundary aabb;
    std::vector<int> triangles;
    std::unique_ptr<std::array<Octree, 8>> nodes;
};
