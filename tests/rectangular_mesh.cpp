#include "waveguide/rectangular_mesh.h"

#include "common/scene_data.h"

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
    SceneData scene_data(OBJ_PATH_BAD_BOX, MAT_PATH_BAD_BOX);
    MeshBoundary boundary(scene_data);
    RectangularMesh mesh(boundary, 0.05, Vec3f());
}
