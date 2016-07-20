#pragma once

#include "boundaries.h"
#include "geometric.h"
#include "scene_data.h"

#include "common/aligned/vector.h"

class Octree final {
public:
    Octree() = default;
    Octree(const CopyableSceneData& mesh_boundary,
           int max_depth,
           float padding = 0);
    Octree(const CopyableSceneData& mesh_boundary,
           int max_depth,
           const aligned::vector<int>& to_test,
           const CuboidBoundary& aabb);

    CuboidBoundary get_aabb() const;
    bool has_nodes() const;
    const std::array<Octree, 8>& get_nodes() const;
    const aligned::vector<int>& get_triangles() const;

    int get_side() const;

    aligned::vector<const Octree*> intersect(const geo::Ray& ray) const;
    const Octree& get_surrounding_leaf(const glm::vec3& v) const;

private:
    CuboidBoundary aabb;
    aligned::vector<int> triangles;
    std::unique_ptr<std::array<Octree, 8>> nodes;
};
