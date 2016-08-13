#include "raytracer/random_directions.h"
#include "raytracer/raytracer.h"

#include "common/conversions.h"
#include "common/spatial_division/voxel_collection.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

TEST(voxel, construct) {
    const scene_data scene(OBJ_PATH);
    const voxelised_scene_data voxelised(
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1}));
}

TEST(voxel, walk) {
    const scene_data scene(OBJ_PATH);
    const voxelised_scene_data voxelised(
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1}));

    const auto rays = 1000;
    const auto directions = raytracer::get_random_directions(rays);
    for (const auto& i : directions) {
        geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));
        bool has_triangles{false};
        traverse(voxelised.get_voxels(),
                 ray,
                 [&](const auto&, const auto& items, float, float) {
                     if (!items.empty()) {
                         has_triangles = true;
                     }
                     return false;
                 });
        ASSERT_TRUE(has_triangles);
    }
}

static constexpr auto bench_rays = 1 << 14;

TEST(voxel, old) {
    const scene_data scene(OBJ_PATH);

    const auto v = scene.get_vertices();
    const auto ind = scene.compute_triangle_indices();

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));
        geo::ray_triangle_intersection(ray, ind, scene.get_triangles(), v);
    }
}

TEST(voxel, new) {
    const scene_data scene(OBJ_PATH);
    const voxelised_scene_data voxelised(
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1}));

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        const geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));
        intersects(voxelised, ray);
    }
}

TEST(voxel, intersect) {
    const scene_data scene(OBJ_PATH);
    const voxelised_scene_data voxelised(
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1}));

    const auto v = scene.get_vertices();
    const auto ind = scene.compute_triangle_indices();

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        const geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));

        const auto inter_0 = geo::ray_triangle_intersection(
                ray, ind, scene.get_triangles(), v);
        const auto inter_1 = intersects(voxelised, ray);

        ASSERT_EQ(inter_0, inter_1);
    }
}

TEST(voxel, flatten) {
    const scene_data scene(OBJ_PATH);
    const voxelised_scene_data voxelised(
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1}));

    const auto f = get_flattened(voxelised.get_voxels());
}
