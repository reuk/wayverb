#pragma once

#include "boundaries.h"
#include "geometric.h"
#include "scene_data.h"

#include "common/aligned/vector.h"

class octree final {
public:
    octree() = default;
    octree(const copyable_scene_data& mesh_boundary,
           size_t max_depth,
           float padding = 0);
    octree(const copyable_scene_data& mesh_boundary,
           size_t max_depth,
           const aligned::vector<size_t>& to_test,
           const box& aabb);

    box get_aabb() const;
    bool has_nodes() const;
    const std::array<octree, 8>& get_nodes() const;
    const aligned::vector<size_t>& get_triangles() const;

    size_t get_side() const;

    aligned::vector<const octree*> intersect(const geo::ray& ray) const;
    const octree& get_surrounding_leaf(const glm::vec3& v) const;

private:
    box aabb;
    aligned::vector<size_t> triangles;
    std::unique_ptr<std::array<octree, 8>> nodes;
};
