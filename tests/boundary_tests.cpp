#include "raytracer/raytracer.h"

#include "waveguide/mesh_boundary.h"

#include "common/boundaries.h"
#include "common/conversions.h"

#include "gtest/gtest.h"

#include <algorithm>

#ifndef OBJ_PATH_TUNNEL
#define OBJ_PATH_TUNNEL ""
#endif

#ifndef OBJ_PATH_BEDROOM
#define OBJ_PATH_BEDROOM ""
#endif

/*
bool naive_test(const MeshBoundary& boundary, const glm::vec3& vec) {
    geo::Ray ray(vec, glm::vec3(0, 0, 1));
    aligned::vector<float> distances;
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
    scene_data scene_data(OBJ_PATH_TUNNEL);
    mesh_boundary boundary(scene_data);

    auto centre = ::centre(boundary.get_aabb());
    ASSERT_TRUE(boundary.inside(centre));

    auto dist = 100;
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(dist, 0, 0)));
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(-dist, 0, 0)));
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(0, dist, 0)));
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(0, -dist, 0)));
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(0, 0, dist)));
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(0, 0, -dist)));

    for (const auto& i : {
                 glm::vec3(0.679999828, -1.18999958, -52.5300026),
                 glm::vec3(-0.680000067, 1.19000006, -52.5300026),
                 glm::vec3(-1.36000013, 2.38000011, -52.7000008),
                 glm::vec3(1.36000013, 2.38000011, -52.7000008),
                 glm::vec3(0.679999828, -1.18999958, -52.1900024),
                 glm::vec3(-0.680000067, 1.19000006, -52.1900024),
         }) {
        ASSERT_FALSE(inside(boundary.get_aabb(), i));
        ASSERT_FALSE(boundary.inside(i));
    }
}

TEST(boundary, bedroom) {
    scene_data scene_data(OBJ_PATH_BEDROOM);
    mesh_boundary boundary(scene_data);

    auto centre = ::centre(boundary.get_aabb());
    ASSERT_TRUE(boundary.inside(centre));

    auto dist = 100;
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(dist, 0, 0)));
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(-dist, 0, 0)));
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(0, dist, 0)));
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(0, -dist, 0)));
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(0, 0, dist)));
    ASSERT_FALSE(boundary.inside(centre + glm::vec3(0, 0, -dist)));

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
        ASSERT_FALSE(inside(boundary.get_aabb(), i));
        ASSERT_FALSE(boundary.inside(i));
    }
}
