#pragma once

#include "common/aligned/vector.h"
#include "common/geo/box.h"
#include "common/geo/triangle_vec.h"
#include "common/triangle.h"
#include "common/voxel_collection.h"

#include "glm/glm.hpp"

class copyable_scene_data;

namespace waveguide {

/// A boundary which allows for fast inside/outside testing.
class mesh_boundary : public boundary {
public:
    explicit mesh_boundary(const copyable_scene_data& sd);

    bool inside(const glm::vec3& v) const override;
    geo::box get_aabb() const override;

    const copyable_scene_data& get_scene_data() const;

private:
    glm::ivec2 hash_point(const glm::vec2& v) const;
    const aligned::vector<size_t>& get_references(const glm::ivec2& i) const;

    copyable_scene_data scene_data;
    voxel_collection<2> triangle_references;
    glm::vec2 cell_size;

    static const aligned::vector<size_t> empty;
};

}  // namespace waveguide
