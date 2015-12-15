#include "bsp.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef MAT_PATH
#define MAT_PATH ""
#endif

void print_nodes(const Octree & o, int level = 0) {
    if (o.get_nodes().empty()) {
        std::cout << "** octree node **" << std::endl;
        for (const auto & i : o.get_triangles()) {
            std::cout << i << std::endl;
        }
    } else {
        for (const auto & i : o.get_nodes()) {
            print_nodes(i, level + 1);
        }
    }
}

TEST(constructor, constructor) {
    SceneData scene_data(OBJ_PATH, MAT_PATH);
    MeshBoundary mesh_boundary(scene_data);
    Octree octree(mesh_boundary, 4);
    print_nodes(octree);
}
