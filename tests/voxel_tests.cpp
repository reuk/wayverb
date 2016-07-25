#include "raytracer/raytracer.h"

#include "common/conversions.h"
#include "common/voxel_collection.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

TEST(voxel, construct) {
    SceneData scene_data(OBJ_PATH);
    Octree octree(scene_data, 2);
    VoxelCollection voxel(octree);
}

class T : public VoxelCollection::TraversalCallback {
public:
    geo::Intersection operator()(
            const geo::Ray& ray,
            const aligned::vector<size_t>& triangles) override {
        if (!triangles.empty()) {
            b = true;
        }
        return geo::Intersection();
    }

    bool get_has_triangles() const { return b; }

private:
    bool b{false};
};

TEST(voxel, walk) {
    SceneData scene_data(OBJ_PATH);
    Octree octree(scene_data, 4, 0.1);
    VoxelCollection voxel(octree);

    auto rays       = 100;
    auto directions = raytracer::get_random_directions(rays);
    for (const auto& i : directions) {
        T t;
        geo::Ray ray(glm::vec3(0, 1, 0), to_vec3f(i));
        voxel.traverse(ray, t);
        ASSERT_TRUE(t.get_has_triangles());
    }
}

static constexpr auto bench_rays = 1 << 14;

TEST(voxel, old) {
    SceneData scene_data(OBJ_PATH);

    auto v = scene_data.get_converted_vertices();
    aligned::vector<size_t> ind(scene_data.get_triangles().size());
    std::iota(ind.begin(), ind.end(), 0);

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        geo::Ray ray(glm::vec3(0, 1, 0), to_vec3f(i));
        geo::ray_triangle_intersection(ray, ind, scene_data.get_triangles(), v);
    }
}

TEST(voxel, new) {
    SceneData scene_data(OBJ_PATH);
    Octree octree(scene_data, 4, 0.1);
    VoxelCollection voxel(octree);

    VoxelCollection::TriangleTraversalCallback t(scene_data);

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        geo::Ray ray(glm::vec3(0, 1, 0), to_vec3f(i));

        voxel.traverse(ray, t);
    }
}

TEST(voxel, intersect) {
    SceneData scene_data(OBJ_PATH);
    Octree octree(scene_data, 4, 0.1);
    VoxelCollection voxel(octree);

    VoxelCollection::TriangleTraversalCallback t(scene_data);

    auto v = scene_data.get_converted_vertices();
    aligned::vector<size_t> ind(scene_data.get_triangles().size());
    std::iota(ind.begin(), ind.end(), 0);

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        geo::Ray ray(glm::vec3(0, 1, 0), to_vec3f(i));

        auto inter_0 = geo::ray_triangle_intersection(
                ray, ind, scene_data.get_triangles(), v);
        auto inter_1 = voxel.traverse(ray, t);

        ASSERT_EQ(inter_0, inter_1);
    }
}

TEST(voxel, flatten) {
    SceneData scene_data(OBJ_PATH);
    Octree octree(scene_data, 5, 0.1);
    VoxelCollection voxel(octree);

    auto f = voxel.get_flattened();
}
