#include "waveguide/mesh/model.h"

#include "common/spatial_division/voxelised_scene_data.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

TEST(mesh_setup, setup) {
    const scene_data scene(OBJ_PATH_BEDROOM);
    const voxelised_scene_data boundary(scene, 5, scene.get_aabb());

    const compute_context cc;
    const auto m = waveguide::mesh::compute_model(
            cc.get_context(), cc.get_device(), boundary, 0.1);
}
