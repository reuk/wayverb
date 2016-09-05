#include "common/reverb_time.h"
#include "common/scene_data_loader.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef OBJ_PATH_TUNNEL
#define OBJ_PATH_TUNNEL ""
#endif

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

#ifndef OBJ_PATH_BAD_BOX
#define OBJ_PATH_BAD_BOX ""
#endif

TEST(reverb_time, triangles_are_oriented) {
    {
        aligned::vector<triangle> triangles{{0, 0, 1, 2}};
        ASSERT_TRUE(triangles_are_oriented(triangles.begin(), triangles.end()));
    }
    {
        aligned::vector<triangle> triangles{{0, 0, 1, 2}, {0, 0, 2, 3}};
        ASSERT_TRUE(triangles_are_oriented(triangles.begin(), triangles.end()));
    }
    {
        aligned::vector<triangle> triangles{{0, 0, 1, 2}, {0, 0, 3, 2}};
        ASSERT_FALSE(
                triangles_are_oriented(triangles.begin(), triangles.end()));
    }

    for (const auto& fname :
         {OBJ_PATH, OBJ_PATH_TUNNEL, OBJ_PATH_BEDROOM, OBJ_PATH_BAD_BOX}) {
        const scene_data_loader loader{fname};
        const auto& triangles{loader.get_scene_data().get_triangles()};
        ASSERT_TRUE(triangles_are_oriented(triangles.begin(), triangles.end()));
    }

    {
        const geo::box box{glm::vec3{-1}, glm::vec3{1}};
        const auto scene{geo::get_scene_data(box)};
        const auto& triangles{scene.get_triangles()};
        ASSERT_TRUE(triangles_are_oriented(triangles.begin(), triangles.end()));
    }
}
