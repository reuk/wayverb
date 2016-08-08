#pragma once

#include "common/aligned/vector.h"
#include "common/boundaries.h"
#include "common/triangle.h"
#include "common/triangle_vec.h"
#include "common/voxel_collection.h"

#include "glm/glm.hpp"

class copyable_scene_data;

/// A boundary which allows for fast inside/outside testing.
class mesh_boundary : public boundary {
public:
    explicit mesh_boundary(const copyable_scene_data& sd);

    bool inside(const glm::vec3& v) const override;
    box<3> get_aabb() const override;

    aligned::vector<size_t> get_triangle_indices() const;

    glm::ivec2 hash_point(const glm::vec3& v) const;
    const aligned::vector<size_t>& get_references(int x, int y) const;
    const aligned::vector<size_t>& get_references(const glm::ivec2& i) const;

    const aligned::vector<triangle>& get_triangles() const;
    const aligned::vector<glm::vec3>& get_vertices() const;
    const aligned::vector<surface>& get_surfaces() const;

private:
    aligned::vector<triangle> triangles;
    aligned::vector<glm::vec3> vertices;
    aligned::vector<surface> surfaces;
    box<3> boundary;

    voxel_collection<2> triangle_references;

    glm::vec2 cell_size;

    static const aligned::vector<size_t> empty;
};
