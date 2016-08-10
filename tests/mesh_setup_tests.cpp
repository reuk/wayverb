#include "waveguide/mesh_boundary.h"
#include "waveguide/mesh_setup.h"

#include "common/scene_data.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

TEST(mesh_setup, setup) {
    compute_context cc;
    waveguide::mesh_boundary boundary(scene_data(OBJ_PATH_BEDROOM));

    const auto nodes = waveguide::mesh_setup::do_mesh_setup(
            cc.get_context(), cc.get_device(), boundary, 0.1, glm::vec3{0});
}
