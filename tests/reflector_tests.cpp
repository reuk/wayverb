#include "raytracer/reflector.h"

#include "common/conversions.h"
#include "common/geo/box.h"
#include "common/map_to_vector.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "glm/glm.hpp"

#include "gtest/gtest.h"

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

const auto badly_behaved_directions{aligned::vector<glm::vec3>{
        glm::vec3{0.00200532563, -0.903969287, 0.427592695},
        glm::vec3{0.473911583, -0.84311819, -0.25408566},
        glm::vec3{-0.156015098, -0.955536007, -0.250220358},
}};

TEST(reflector, locations) {
    const auto box{geo::box{glm::vec3(0, 0, 0), glm::vec3(4, 3, 6)}};
    const auto scene{geo::get_scene_data(box)};
    const auto voxelised{voxelised_scene_data{
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1})}};
    const auto cc{compute_context{}};
    const auto buffers{scene_buffers{cc.get_context(), voxelised}};

    const auto source{glm::vec3{1, 2, 1}};
    const auto receiver{glm::vec3{2, 1, 2}};

    const auto directions{raytracer::get_random_directions(1000)};
    //    const auto directions = badly_behaved_directions;

    auto reflector{raytracer::reflector{
            cc.get_context(), cc.get_device(), source, receiver, directions}};

    const auto reflections{reflector.run_step(buffers)};

    ASSERT_TRUE(proc::any_of(reflections,
                             [](const auto& i) { return i.keep_going; }));
    ASSERT_TRUE(proc::all_of(reflections,
                             [](const auto& i) { return i.keep_going; }));

    const auto intersections{map_to_vector(directions, [&](const auto& i) {
        return intersects(voxelised, geo::ray{source, i});
    })};

    for (auto i = 0; i != directions.size(); ++i) {
        ASSERT_TRUE(intersections[i]);
        const auto cpu_position{source +
                                (directions[i] * intersections[i]->inter.t)};
        const auto gpu_position{to_vec3(reflections[i].position)};
        ASSERT_TRUE(nearby(gpu_position, cpu_position, 0.00001));
    }
}
