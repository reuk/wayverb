#include "common/octree.h"
#include "common/serialize/boundaries.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

void print_nodes(const octree& o, int level = 0) {
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
    scene_data scene_data(OBJ_PATH);
    octree octree(scene_data, 5);
    //    print_nodes(octree);
}

TEST(octree, surrounding) {
    scene_data scene_data(OBJ_PATH);

    auto a = {
            glm::vec3(-1, -1, -1),
            glm::vec3(-1, -1, 1),
            glm::vec3(-1, 1, -1),
            glm::vec3(-1, 1, 1),
            glm::vec3(1, -1, -1),
            glm::vec3(1, -1, 1),
            glm::vec3(1, 1, -1),
            glm::vec3(1, 1, 1),
    };

    {
        octree octree(scene_data, 1);
        auto c = ::centre(octree.get_aabb());

        for (const auto& i : a) {
            auto pt = c + i;
            ASSERT_TRUE(inside(octree.get_surrounding_leaf(pt).get_aabb(), pt))
                    << i;
        }
    }

    {
        octree octree(scene_data, 4);
        auto c = ::centre(octree.get_aabb());

        for (const auto& i : a) {
            auto pt = c + i;
            ASSERT_TRUE(inside(octree.get_surrounding_leaf(pt).get_aabb(), pt))
                    << i;
        }
    }
}
