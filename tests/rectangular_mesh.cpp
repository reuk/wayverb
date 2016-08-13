#include "common/spatial_division/voxelised_scene_data.h"
#include "waveguide/mesh/model.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH_TUNNEL
#define OBJ_PATH_TUNNEL ""
#endif

#ifndef MAT_PATH_TUNNEL
#define MAT_PATH_TUNNEL ""
#endif

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

#ifndef MAT_PATH_BEDROOM
#define MAT_PATH_BEDROOM ""
#endif

#ifndef OBJ_PATH_BAD_BOX
#define OBJ_PATH_BAD_BOX ""
#endif

#ifndef MAT_PATH_BAD_BOX
#define MAT_PATH_BAD_BOX ""
#endif

TEST(mesh_classification, badbox) {
    const compute_context cc;
    const scene_data scene_data(OBJ_PATH_BAD_BOX);
    const voxelised_scene_data voxelised(
            scene_data, 5, util::padded(scene_data.get_aabb(), glm::vec3{0.1}));
    const auto m = waveguide::mesh::compute_model(
            cc.get_context(), cc.get_device(), voxelised, 0.05);
}
