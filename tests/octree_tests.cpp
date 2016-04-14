#include "octree.h"

#include "gtest/gtest.h"

#include "boundaries_serialize.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef MAT_PATH
#define MAT_PATH ""
#endif

void print_nodes(const Octree& o, int level = 0) {
    if (o.get_nodes().empty()) {
        std::cout << "** octree node **" << std::endl;
        for (const auto& i : o.get_triangles()) {
            std::cout << i << std::endl;
        }
    } else {
        for (const auto& i : o.get_nodes()) {
            print_nodes(i, level + 1);
        }
    }
}

TEST(octree, constructor) {
    SceneData scene_data(OBJ_PATH, MAT_PATH);
    Octree octree(scene_data, 5);
    //    print_nodes(octree);
}

TEST(octree, flatten) {
    SceneData scene_data(OBJ_PATH, MAT_PATH);
    Octree octree(scene_data, 5);
    auto flattened = octree.get_flattened();
}

TEST(octree, surrounding) {
    SceneData scene_data(OBJ_PATH, MAT_PATH);

    auto a = {
        Vec3f(-1, -1, -1),
        Vec3f(-1, -1, 1),
        Vec3f(-1, 1, -1),
        Vec3f(-1, 1, 1),
        Vec3f(1, -1, -1),
        Vec3f(1, -1, 1),
        Vec3f(1, 1, -1),
        Vec3f(1, 1, 1),
    };

    {
        Octree octree(scene_data, 1);
        auto c = octree.get_aabb().centre();

        for (const auto& i : a) {
            auto pt = c + i;
            ASSERT_TRUE(octree.get_surrounding_leaf(pt).get_aabb().inside(pt))
                << i;
        }
    }

    {
        Octree octree(scene_data, 4);
        auto c = octree.get_aabb().centre();

        for (const auto& i : a) {
            auto pt = c + i;
            ASSERT_TRUE(octree.get_surrounding_leaf(pt).get_aabb().inside(pt))
                << i;
        }
    }
}
