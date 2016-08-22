#include "waveguide/mesh/model.h"

#include "common/spatial_division/voxelised_scene_data.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

namespace {

auto get_voxelised(const copyable_scene_data& scene) {
    return voxelised_scene_data{
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1})};
}

TEST(mesh_setup, setup) {
    const auto boundary{get_voxelised(scene_data{OBJ_PATH_BEDROOM})};
    const auto m{waveguide::mesh::compute_model(
            compute_context{}, boundary, 0.1, 340)};
}

}  // namespace
