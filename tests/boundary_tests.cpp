#include "boundaries.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH_TUNNEL
#define OBJ_PATH_TUNNEL ""
#endif

#ifndef MAT_PATH_TUNNEL
#define MAT_PATH_TUNNEL ""
#endif

TEST(boundary, mesh) {
    SceneData scene_data(OBJ_PATH_TUNNEL, MAT_PATH_TUNNEL);
    MeshBoundary boundary(scene_data);

    ASSERT_TRUE(boundary.inside(Vec3f(0, 1, 0)));
}
