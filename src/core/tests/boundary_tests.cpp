#include "core/conversions.h"
#include "core/scene_data_loader.h"
#include "core/spatial_division/voxelised_scene_data.h"

#include "gtest/gtest.h"

#include <algorithm>

#ifndef OBJ_PATH_TUNNEL
#define OBJ_PATH_TUNNEL ""
#endif

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

using namespace wayverb::core;

TEST(boundary, tunnel) {
    const auto boundary{make_voxelised_scene_data(
            scene_data_loader{OBJ_PATH_TUNNEL}.get_scene_data(), 5, 2.0f)};

    const auto cent{centre(boundary.get_voxels().get_aabb())};

    ASSERT_EQ(*count_intersections(boundary,
                                   geo::ray{cent,
                                            glm::vec3{-0.108882785,
                                                      0.075211525,
                                                      0.991205215}}),
              1);

    {
        const auto dist{100};
        ASSERT_FALSE(inside(boundary, cent + glm::vec3(dist, 0, 0)));
        ASSERT_FALSE(inside(boundary, cent + glm::vec3(-dist, 0, 0)));
        ASSERT_FALSE(inside(boundary, cent + glm::vec3(0, dist, 0)));
        ASSERT_FALSE(inside(boundary, cent + glm::vec3(0, -dist, 0)));
        ASSERT_FALSE(inside(boundary, cent + glm::vec3(0, 0, dist)));
        ASSERT_FALSE(inside(boundary, cent + glm::vec3(0, 0, -dist)));
    }

    for (const auto& i : {
                 glm::vec3(0.679999828, -1.18999958, -52.5300026),
                 glm::vec3(-0.680000067, 1.19000006, -52.5300026),
                 glm::vec3(-1.36000013, 2.38000011, -52.7000008),
                 glm::vec3(1.36000013, 2.38000011, -52.7000008),
                 glm::vec3(0.679999828, -1.18999958, -52.1900024),
                 glm::vec3(-0.680000067, 1.19000006, -52.1900024),
         }) {
        ASSERT_FALSE(inside(boundary.get_voxels().get_aabb(), i));
        ASSERT_FALSE(inside(boundary, i));
    }

    ASSERT_TRUE(inside(boundary, cent));
    {
        const auto dist{0.1};
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(dist, 0, 0)));
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(-dist, 0, 0)));
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(0, dist, 0)));
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(0, -dist, 0)));
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(0, 0, dist)));
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(0, 0, -dist)));
    }
}

TEST(boundary, bedroom) {
    const auto boundary{make_voxelised_scene_data(
            scene_data_loader{OBJ_PATH_BEDROOM}.get_scene_data(), 5, 0.1f)};

    const auto cent{centre(boundary.get_voxels().get_aabb())};

    const auto dist{100};
    ASSERT_FALSE(inside(boundary, cent + glm::vec3(dist, 0, 0)));
    ASSERT_FALSE(inside(boundary, cent + glm::vec3(-dist, 0, 0)));
    ASSERT_FALSE(inside(boundary, cent + glm::vec3(0, dist, 0)));
    ASSERT_FALSE(inside(boundary, cent + glm::vec3(0, -dist, 0)));
    ASSERT_FALSE(inside(boundary, cent + glm::vec3(0, 0, dist)));
    ASSERT_FALSE(inside(boundary, cent + glm::vec3(0, 0, -dist)));

    for (const auto& i : {
                 glm::vec3(1.70000005, -1.70000005, -4.11999989),
                 glm::vec3(-1.70000005, -1.19000006, -3.94999981),
                 glm::vec3(1.70000005, -1.36000001, -4.11999989),
                 glm::vec3(-1.70000005, 1.01999998, -4.11999989),
                 glm::vec3(1.70000005, -1.53000009, -3.94999981),
                 glm::vec3(1.70000005, -1.19000006, -3.94999981),
                 glm::vec3(1.70000005, -0.850000023, -3.94999981),
                 glm::vec3(1.70000005, -0.50999999, -3.94999981),
                 glm::vec3(1.70000005, -0.169999838, -3.94999981),
                 glm::vec3(1.70000005, 0.170000076, -3.94999981),
                 glm::vec3(1.70000005, 0.50999999, -3.94999981),
                 glm::vec3(1.70000005, 0.850000143, -3.94999981),
                 glm::vec3(1.70000005, 1.19000006, -3.94999981),
                 glm::vec3(-1.70000005, -1.01999998, -3.77999997),
                 glm::vec3(-1.70000005, -0.680000067, -3.77999997),
                 glm::vec3(-1.70000005, -0.339999914, -3.77999997),
                 glm::vec3(-1.70000005, 0, -3.77999997),
                 glm::vec3(-1.70000005, 0.680000067, -3.77999997),
         }) {
        ASSERT_FALSE(inside(boundary.get_voxels().get_aabb(), i));
        ASSERT_FALSE(inside(boundary, i));
    }

    ASSERT_TRUE(inside(boundary, cent));
    {
        const auto dist{0.1};
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(dist, 0, 0)));
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(-dist, 0, 0)));
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(0, dist, 0)));
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(0, -dist, 0)));
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(0, 0, dist)));
        ASSERT_TRUE(inside(boundary, cent + glm::vec3(0, 0, -dist)));
    }
}
