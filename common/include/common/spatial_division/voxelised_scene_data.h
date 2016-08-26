#pragma once

#include "common/scene_data.h"
#include "common/spatial_division/voxel_collection.h"

class voxelised_scene_data final {
public:
    //  invariant:
    //  The 'voxels' structure holds references/indexes to valid triangles in
    //  the 'scene_data' structure.

    voxelised_scene_data(scene_data, size_t octree_depth, const geo::box& aabb);

    const scene_data& get_scene_data() const;
    const voxel_collection<3>& get_voxels() const;

    //  We can allow modifying surfaces without violating the invariant.
    void set_surfaces(const aligned::vector<scene_data::material>& materials);
    void set_surfaces(const aligned::map<std::string, surface>& surfaces);
    void set_surface(const scene_data::material& material);

    void set_surfaces(const surface& surface);

private:
    scene_data scene;
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
