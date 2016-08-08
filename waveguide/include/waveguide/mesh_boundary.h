#pragma once

#include "common/boundaries.h"
#include "common/triangle.h"
#include "common/triangle_vec.h"

#include "common/aligned/vector.h"

#include "glm/glm.hpp"

class copyable_scene_data;

/// A boundary which allows for fast inside/outside testing.
class mesh_boundary : public boundary {
public:
    mesh_boundary(const aligned::vector<triangle>& triangles,
                  const aligned::vector<glm::vec3>& vertices,
                  const aligned::vector<surface>& surfaces);
    explicit mesh_boundary(const copyable_scene_data& sd);
    bool inside(const glm::vec3& v) const override;
    box<3> get_aabb() const override;

    aligned::vector<size_t> get_triangle_indices() const;

    using reference_store = aligned::vector<uint32_t>;

    glm::ivec3 hash_point(const glm::vec3& v) const;
    const reference_store& get_references(int x, int y) const;
    const reference_store& get_references(const glm::ivec3& i) const;

    static constexpr int DIVISIONS = 1024;

    const aligned::vector<triangle>& get_triangles() const;
    const aligned::vector<glm::vec3>& get_vertices() const;
    const aligned::vector<surface>& get_surfaces() const;
    glm::vec3 get_cell_size() const;

private:
    using hash_table = aligned::vector<aligned::vector<reference_store>>;

    hash_table compute_triangle_references() const;

    aligned::vector<triangle> triangles;
    aligned::vector<glm::vec3> vertices;
    aligned::vector<surface> surfaces;
    box<3> boundary;
    glm::vec3 cell_size;
    hash_table triangle_references;

    static const reference_store empty_reference_store;
};
