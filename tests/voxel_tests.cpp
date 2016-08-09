#include "raytracer/raytracer.h"

#include "common/conversions.h"
#include "common/voxel_collection.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

TEST(voxel, construct) {
    scene_data scene_data(OBJ_PATH);
    voxel_collection<3> voxel(octree_from_scene_data(scene_data, 4, 0.1));
}

TEST(voxel, walk) {
    scene_data scene_data(OBJ_PATH);
    voxel_collection<3> voxel(octree_from_scene_data(scene_data, 4, 0.1));

    auto rays = 100;
    auto directions = raytracer::get_random_directions(rays);
    for (const auto& i : directions) {
        geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));
        bool has_triangles{false};
        traverse(voxel, ray, [&](const auto& ray, const auto& items) {
            if (!items.empty()) {
                has_triangles = true;
            }
            return geo::intersection();
        });
        ASSERT_TRUE(has_triangles);
    }
}

static constexpr auto bench_rays = 1 << 14;

TEST(voxel, old) {
    scene_data scene_data(OBJ_PATH);

    const auto v = scene_data.get_converted_vertices();
    const auto ind = scene_data.get_triangle_indices();

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));
        geo::ray_triangle_intersection(ray, ind, scene_data.get_triangles(), v);
    }
}

TEST(voxel, new) {
    scene_data scene_data(OBJ_PATH);
    voxel_collection<3> voxel(octree_from_scene_data(scene_data, 4, 0.1));

    triangle_traversal_callback t(scene_data);

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));

        traverse(voxel, ray, t);
    }
}

TEST(voxel, intersect) {
    scene_data scene_data(OBJ_PATH);
    voxel_collection<3> voxel(octree_from_scene_data(scene_data, 4, 0.1));

    triangle_traversal_callback t(scene_data);

    const auto v = scene_data.get_converted_vertices();
    const auto ind = scene_data.get_triangle_indices();

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));

        auto inter_0 = geo::ray_triangle_intersection(
                ray, ind, scene_data.get_triangles(), v);
        auto inter_1 = traverse(voxel, ray, t);

        ASSERT_EQ(inter_0, inter_1);
    }
}

TEST(voxel, flatten) {
    scene_data scene_data(OBJ_PATH);
    voxel_collection<3> voxel(octree_from_scene_data(scene_data, 4, 0.1));

    const auto f = get_flattened(voxel);
}
