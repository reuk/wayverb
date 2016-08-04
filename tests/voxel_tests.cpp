#include "raytracer/raytracer.h"

#include "common/conversions.h"
#include "common/voxel_collection.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

TEST(voxel, construct) {
    scene_data scene_data(OBJ_PATH);
    octree octree(scene_data, 2);
    voxel_collection voxel(octree);
}

class T : public voxel_collection::traversal_callback {
public:
    geo::intersection operator()(
            const geo::ray& ray,
            const aligned::vector<size_t>& triangles) const override {
        if (!triangles.empty()) {
            b = true;
        }
        return geo::intersection();
    }

    bool get_has_triangles() const { return b; }

private:
    mutable bool b{false};
};

TEST(voxel, walk) {
    scene_data scene_data(OBJ_PATH);
    octree octree(scene_data, 4, 0.1);
    voxel_collection voxel(octree);

    auto rays       = 100;
    auto directions = raytracer::get_random_directions(rays);
    for (const auto& i : directions) {
        T t;
        geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));
        voxel.traverse(ray, t);
        ASSERT_TRUE(t.get_has_triangles());
    }
}

static constexpr auto bench_rays = 1 << 14;

TEST(voxel, old) {
    scene_data scene_data(OBJ_PATH);

    auto v = scene_data.get_converted_vertices();
    aligned::vector<size_t> ind(scene_data.get_triangles().size());
    std::iota(ind.begin(), ind.end(), 0);

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));
        geo::ray_triangle_intersection(ray, ind, scene_data.get_triangles(), v);
    }
}

TEST(voxel, new) {
    scene_data scene_data(OBJ_PATH);
    octree octree(scene_data, 4, 0.1);
    voxel_collection voxel(octree);

    voxel_collection::triangle_traversal_callback t(scene_data);

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));

        voxel.traverse(ray, t);
    }
}

TEST(voxel, intersect) {
    scene_data scene_data(OBJ_PATH);
    octree octree(scene_data, 4, 0.1);
    voxel_collection voxel(octree);

    voxel_collection::triangle_traversal_callback t(scene_data);

    auto v = scene_data.get_converted_vertices();
    aligned::vector<size_t> ind(scene_data.get_triangles().size());
    std::iota(ind.begin(), ind.end(), 0);

    for (const auto& i : raytracer::get_random_directions(bench_rays)) {
        geo::ray ray(glm::vec3(0, 1, 0), to_vec3(i));

        auto inter_0 = geo::ray_triangle_intersection(
                ray, ind, scene_data.get_triangles(), v);
        auto inter_1 = voxel.traverse(ray, t);

        ASSERT_EQ(inter_0, inter_1);
    }
}

TEST(voxel, flatten) {
    scene_data scene_data(OBJ_PATH);
    octree octree(scene_data, 5, 0.1);
    voxel_collection voxel(octree);

    auto f = voxel.get_flattened();
}
