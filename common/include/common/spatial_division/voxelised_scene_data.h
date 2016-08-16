#pragma once

#include "common/scene_data.h"
#include "common/spatial_division/voxel_collection.h"

class voxelised_scene_data final {
public:
    voxelised_scene_data(const copyable_scene_data& scene_data,
                         size_t octree_depth,
                         const geo::box& aabb);

    const copyable_scene_data& get_scene_data() const;
    const voxel_collection<3>& get_voxels() const;

private:
    copyable_scene_data scene_data;
    voxel_collection<3> voxels;
};

//----------------------------------------------------------------------------//

std::experimental::optional<intersection> intersects(
        const voxelised_scene_data& voxelised,
        const geo::ray& ray,
        size_t to_ignore = ~size_t{0});

std::experimental::optional<size_t> count_intersections(
        const voxelised_scene_data& voxelised, const geo::ray& ray);

bool inside(const voxelised_scene_data& voxelised, const glm::vec3& pt);
