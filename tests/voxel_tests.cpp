#include "voxel_collection.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef MAT_PATH
#define MAT_PATH ""
#endif

TEST(voxel, construct) {
    SceneData scene_data(OBJ_PATH, MAT_PATH);
    Octree octree(scene_data, 2);
    VoxelCollection voxel(octree);
}
