#include "waveguide/mesh.h"

#include "common/spatial_division/voxelised_scene_data.h"
#include "common/scene_data_loader.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

namespace {

auto get_voxelised(const scene_data& scene) {
    return voxelised_scene_data{
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1})};
}

TEST(mesh_setup, setup) {
    const auto boundary{get_voxelised(
            scene_data_loader{OBJ_PATH_BEDROOM}.get_scene_data())};
    const auto m{
            waveguide::compute_mesh(compute_context{}, boundary, 0.1, 340)};
}

}  // namespace
