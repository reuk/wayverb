#include "boundaries.h"
#include "rayverb.h"
#include "conversions.h"

#include "gtest/gtest.h"

#include <algorithm>

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

/*
bool naive_test(const MeshBoundary& boundary, const Vec3f& vec) {
    geo::Ray ray(vec, Vec3f(0, 0, 1));
    std::vector<float> distances;
    return
        std::count_if(
            boundary.get_triangles().begin(),
            boundary.get_triangles().end(),
            [&ray, &distances, &boundary](const auto& i) {
                auto intersection =
                    triangle_intersection(i, boundary.get_vertices(), ray);
                if (intersection.intersects) {
                    for (auto d : distances) {
                        if (almost_equal(d, intersection.distance, 1)) {
                            distances.push_back(intersection.distance);
                            return false;
                        }
                    }
                    distances.push_back(intersection.distance);
                }
                return intersection.intersects;
            }) %
        2;
}

TEST(boundary, naive) {
    SceneData scene_data(OBJ_PATH_TUNNEL, MAT_PATH_TUNNEL);
    MeshBoundary boundary(scene_data);

    for (const auto & i : get_random_directions(1 << 20)) {
        auto vec = to_vec3f(i);
        ASSERT_EQ(naive_test(boundary, vec), boundary.inside(vec));
    }
}
*/

TEST(boundary, tunnel) {
    SceneData scene_data(OBJ_PATH_TUNNEL, MAT_PATH_TUNNEL);
    MeshBoundary boundary(scene_data);

    auto centre = boundary.get_aabb().get_centre();
    ASSERT_TRUE(boundary.inside(centre));

    auto dist = 100;
    ASSERT_FALSE(boundary.inside(centre + Vec3f(dist, 0, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(-dist, 0, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0, dist, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0, -dist, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0, 0, dist)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0, 0, -dist)));

    for (const auto & i : {
             Vec3f(0.679999828, -1.18999958, -52.5300026),
             Vec3f(-0.680000067, 1.19000006, -52.5300026),
             Vec3f(-1.36000013, 2.38000011, -52.7000008),
             Vec3f(1.36000013, 2.38000011, -52.7000008),
             Vec3f(0.679999828, -1.18999958, -52.1900024),
             Vec3f(-0.680000067, 1.19000006, -52.1900024),
         }) {
        ASSERT_FALSE(boundary.get_aabb().inside(i));
        ASSERT_FALSE(boundary.inside(i));
    }
}

TEST(boundary, bedroom) {
    SceneData scene_data(OBJ_PATH_BEDROOM, MAT_PATH_BEDROOM);
    MeshBoundary boundary(scene_data);

    auto centre = boundary.get_aabb().get_centre();
    ASSERT_TRUE(boundary.inside(centre));

    auto dist = 100;
    ASSERT_FALSE(boundary.inside(centre + Vec3f(dist, 0, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(-dist, 0, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0, dist, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0, -dist, 0)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0, 0, dist)));
    ASSERT_FALSE(boundary.inside(centre + Vec3f(0, 0, -dist)));

    for (const auto & i : {
             Vec3f(1.70000005, -1.70000005, -4.11999989),
             Vec3f(-1.70000005, -1.19000006, -3.94999981),
             Vec3f(1.70000005, -1.36000001, -4.11999989),
             Vec3f(-1.70000005, 1.01999998, -4.11999989),
             Vec3f(1.70000005, -1.53000009, -3.94999981),
             Vec3f(1.70000005, -1.19000006, -3.94999981),
             Vec3f(1.70000005, -0.850000023, -3.94999981),
             Vec3f(1.70000005, -0.50999999, -3.94999981),
             Vec3f(1.70000005, -0.169999838, -3.94999981),
             Vec3f(1.70000005, 0.170000076, -3.94999981),
             Vec3f(1.70000005, 0.50999999, -3.94999981),
             Vec3f(1.70000005, 0.850000143, -3.94999981),
             Vec3f(1.70000005, 1.19000006, -3.94999981),
             Vec3f(-1.70000005, -1.01999998, -3.77999997),
             Vec3f(-1.70000005, -0.680000067, -3.77999997),
             Vec3f(-1.70000005, -0.339999914, -3.77999997),
             Vec3f(-1.70000005, 0, -3.77999997),
             Vec3f(-1.70000005, 0.680000067, -3.77999997),
         }) {
        ASSERT_FALSE(boundary.get_aabb().inside(i));
        ASSERT_FALSE(boundary.inside(i));
    }
}
