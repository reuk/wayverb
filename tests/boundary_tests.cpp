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

    auto centre = boundary.get_aabb().get_centre();
    ASSERT_TRUE(boundary.inside(centre));

    auto dist = 100;
    ASSERT_FALSE(boundary.inside(centre + Vec3f( dist, 0, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(-dist, 0, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0,  dist, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0, -dist, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0, 0,  dist)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0, 0, -dist)));

    auto test_point = [&boundary](auto pt) {
        ASSERT_FALSE(boundary.get_aabb().inside(pt));
        ASSERT_FALSE(boundary.inside(pt));
    };

    test_point(Vec3f(0.679999828, -1.18999958, -52.5300026));
    test_point(Vec3f(-0.680000067, 1.19000006, -52.5300026));
    test_point(Vec3f(-1.36000013, 2.38000011, -52.7000008));
    test_point(Vec3f(1.36000013, 2.38000011, -52.7000008));
    test_point(Vec3f(0.679999828, -1.18999958, -52.1900024));
    test_point(Vec3f(-0.680000067, 1.19000006, -52.1900024));
}
