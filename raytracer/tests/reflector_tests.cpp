#include "raytracer/reflector.h"

#include "common/conversions.h"
#include "common/geo/box.h"
#include "common/map_to_vector.h"
#include "common/spatial_division/voxelised_scene_data.h"

#include "glm/glm.hpp"

#include "gtest/gtest.h"

namespace {
const auto badly_behaved_directions{aligned::vector<glm::vec3>{
        glm::vec3{0.00200532563, -0.903969287, 0.427592695},
        glm::vec3{0.473911583, -0.84311819, -0.25408566},
        glm::vec3{-0.156015098, -0.955536007, -0.250220358},
        glm::vec3{0.567212641, -0.7546525, 0.329802126},
        glm::vec3{0.551898658, -0.809262037, 0.201253086},
}};

const auto badly_behaved_rays{aligned::vector<geo::ray>{
        geo::ray{glm::vec3{2.91376591, 2.99994421, 6},
                 glm::vec3{0.280182242, 0.849850773, -0.446375906}},
        geo::ray{glm::vec3{4, 2.65516424, 5.99998713},
                 glm::vec3{-0.501335502, -0.224737942, 0.835557043}},
        geo::ray{glm::vec3{0.00000566244125, 3, 0.800823509},
                 glm::vec3{-0.148433656, -0.987669587, 0.0497597083}},
        geo::ray{glm::vec3{0, 0.000000596046448, 1.53889632},
                 glm::vec3{0.434765905, -0.869531512, 0.234293744}}}};

auto get_voxelised(scene_data scene) {
    scene.set_surfaces(surface{volume_type{{1, 1, 1, 1, 1, 1, 1, 1}},
                               volume_type{{0, 0, 0, 0, 0, 0, 0, 0}}});
    return voxelised_scene_data{
            scene, 5, util::padded(scene.get_aabb(), glm::vec3{0.1})};
}

struct reflector_fixture : public ::testing::Test {
    const geo::box box{glm::vec3(0, 0, 0), glm::vec3(4, 3, 6)};
    const voxelised_scene_data voxelised{
            get_voxelised(geo::get_scene_data(box))};
    const compute_context cc{};
    const scene_buffers buffers{cc.context, voxelised};

    const glm::vec3 source{1, 2, 1};
    const glm::vec3 receiver{2, 1, 2};

    static constexpr auto speed_of_sound{340};

#define OPT (0)
#if OPT == 0
    const aligned::vector<geo::ray> rays{
            raytracer::get_random_rays(source, 10000)};
#elif OPT == 1
    const aligned::vector<geo::ray> rays{raytracer::get_rays_from_directions(
            source, badly_behaved_directions)};
#elif OPT == 2
    const aligned::vector<geo::ray> rays{badly_behaved_rays};
#endif

    raytracer::reflector reflector{cc, receiver, rays, speed_of_sound};

    auto get_fast_intersections() const {
        const auto rays{reflector.get_rays()};
        const auto reflections{reflector.get_reflections()};
        aligned::vector<std::experimental::optional<intersection>> ret;
        ret.reserve(rays.size());
        for (auto i{0u}; i != rays.size(); ++i) {
            ret.push_back(intersects(
                    voxelised, convert(rays[i]), reflections[i].triangle));
        }
        return ret;
    }

    auto get_slow_intersections() const {
        const auto rays{reflector.get_rays()};
        const auto reflections{reflector.get_reflections()};
        aligned::vector<std::experimental::optional<intersection>> ret;
        ret.reserve(rays.size());
        for (auto i{0u}; i != rays.size(); ++i) {
            ret.push_back(geo::ray_triangle_intersection(
                    convert(rays[i]),
                    voxelised.get_scene_data().get_triangles(),
                    convert(voxelised.get_scene_data().get_vertices()),
                    reflections[i].triangle));
        }
        return ret;
    }
};

TEST_F(reflector_fixture, locations) {
    const auto fast_intersections{get_fast_intersections()};
    const auto slow_intersections{get_slow_intersections()};
    for (auto i{0u}; i != fast_intersections.size(); ++i) {
        ASSERT_EQ(fast_intersections[i], slow_intersections[i]);
    }

    const auto current_rays{reflector.get_rays()};
    const auto reflections{reflector.run_step(buffers)};

    ASSERT_TRUE(proc::any_of(reflections,
                             [](const auto& i) { return i.keep_going; }));
    ASSERT_TRUE(proc::all_of(reflections,
                             [](const auto& i) { return i.keep_going; }));

    for (auto i = 0; i != current_rays.size(); ++i) {
        ASSERT_TRUE(fast_intersections[i]);
        const auto converted{convert(current_rays[i])};
        const auto cpu_position{
                converted.get_position() +
                (converted.get_direction() * fast_intersections[i]->inter.t)};
        const auto gpu_position{to_vec3(reflections[i].position)};
        ASSERT_TRUE(nearby(gpu_position, cpu_position, 0.00001));
    }
}

TEST_F(reflector_fixture, multi_layer_reflections) {
    for (auto i{0u}; i != 10; ++i) {
        const auto prev_reflections{reflector.get_reflections()};
        const auto current_rays{reflector.get_rays()};
        const auto fast_intersections{get_fast_intersections()};
        const auto slow_intersections{get_slow_intersections()};
        for (auto i{0u}; i != fast_intersections.size(); ++i) {
            ASSERT_EQ(fast_intersections[i], slow_intersections[i]);
        }
        const auto reflections{reflector.run_step(buffers)};

        for (auto j{0u}; j != current_rays.size(); ++j) {
            ASSERT_TRUE(reflections[j].keep_going);
            ASSERT_TRUE(fast_intersections[j]);
            const auto converted{convert(current_rays[j])};
            const auto cpu_position{converted.get_position() +
                                    (converted.get_direction() *
                                     fast_intersections[j]->inter.t)};
            const auto gpu_position{to_vec3(reflections[j].position)};
            const auto is_nearby{nearby(gpu_position, cpu_position, 0.00001)};
            if (!is_nearby) {
                std::cout << j << '\n';
                std::cout << reflections[j].triangle << ", "
                          << prev_reflections[j].triangle << '\n';
            }
            ASSERT_TRUE(is_nearby);
        }
    }
}
}  // namespace
