#pragma once

#include "boundaries.h"
#include "float_uint.h"
#include "geometric.h"
#include "scene_data.h"

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
    std::vector<FloatUInt> get_flattened() const;

    int get_side() const;

    std::vector<const Octree*> intersect(const geo::Ray& ray) const;
    const Octree& get_surrounding_leaf(const Vec3f& v) const;

private:
    void fill_flattened(std::vector<FloatUInt>& ret) const;

    CuboidBoundary aabb;
    std::vector<int> triangles;
    std::unique_ptr<std::array<Octree, 8>> nodes;
};
