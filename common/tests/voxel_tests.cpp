#include "common/azimuth_elevation.h"
#include "common/conversions.h"
#include "common/map_to_vector.h"
#include "common/scene_data_loader.h"
#include "common/spatial_division/scene_buffers.h"
#include "common/spatial_division/voxel_collection.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

namespace {
auto get_voxelised(const scene_data& scene) {
    return voxelised_scene_data{
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1})};
}

aligned::vector<scene_data> test_scenes{
        geo::get_scene_data(geo::box{glm::vec3(0, 0, 0), glm::vec3(4, 3, 6)}),
        geo::get_scene_data(geo::box{glm::vec3(0, 0, 0), glm::vec3(3, 3, 3)}),
        scene_data_loader{OBJ_PATH}.get_scene_data()};

TEST(voxel, construct) {
    for (const auto& scene : test_scenes) {
        const auto voxelised{get_voxelised(scene)};
    }
}

TEST(voxel, walk) {
    for (const auto& scene : test_scenes) {
        const auto voxelised{get_voxelised(scene)};

        const auto rays = 1000;
        const auto directions = get_random_directions(rays);
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
}

TEST(voxel, flatten) {
    for (const auto& scene : test_scenes) {
        const auto voxelised{get_voxelised(scene)};
        const auto f = get_flattened(voxelised.get_voxels());
    }
}

void compare(const scene_data& scene) {
    const auto voxelised{get_voxelised(scene)};
    const compute_context cc{};
    const scene_buffers buffers{cc.context, voxelised};

    const glm::vec3 source{1, 2, 1};

    const auto directions{get_random_directions(1000)};

    const auto get_intersections_fast{[&](const auto& directions) {
        return map_to_vector(directions, [&](const auto& i) {
            const auto ray{geo::ray{source, i}};
            return intersects(voxelised, ray);
        });
    }};

    const auto get_intersections_slow{[&](const auto& directions) {
        return map_to_vector(directions, [&](const auto& i) {
            return geo::ray_triangle_intersection(
                    geo::ray{source, i},
                    voxelised.get_scene_data().get_triangles(),
                    convert(voxelised.get_scene_data().get_vertices()));
        });
    }};

    const auto fast_intersections{get_intersections_fast(directions)};
    const auto slow_intersections{get_intersections_slow(directions)};

    for (auto i : slow_intersections) {
        ASSERT_TRUE(i);
    }

    aligned::vector<glm::vec3> problem_directions{};
    std::set<size_t> problem_surfaces{};
    for (auto i{0u}; i != directions.size(); ++i) {
        if (!static_cast<bool>(fast_intersections[i])) {
            problem_directions.push_back(directions[i]);
            problem_surfaces.insert(slow_intersections[i]->index);
        }
    }

    std::cout << "problem directions:\n";
    for (const auto i : problem_directions) {
        std::cout << '{' << i.x << ", " << i.y << ", " << i.z << "}\n";
    }

    std::cout << "problem surfaces:\n";
    for (const auto i : problem_surfaces) {
        std::cout << i << '\n';
    }

    //  now let's try just the problematic directions
    const auto problem_intersections{
            get_intersections_fast(problem_directions)};

    for (auto i{0u}; i != directions.size(); ++i) {
        ASSERT_EQ(static_cast<bool>(fast_intersections[i]),
                  static_cast<bool>(slow_intersections[i]));
        ASSERT_EQ(fast_intersections[i]->index, slow_intersections[i]->index);
    }
}

TEST(voxel, compare) {
    for (const auto& scene : test_scenes) {
        compare(scene);
    }
}
}  // namespace
