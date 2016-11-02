#include "raytracer/reflector.h"

#include "core/conversions.h"
#include "core/geo/box.h"
#include "core/spatial_division/voxelised_scene_data.h"

#include "glm/glm.hpp"

#include "gtest/gtest.h"

namespace {
const auto badly_behaved_directions = util::aligned::vector<glm::vec3>{
        glm::vec3{0.00200532563, -0.903969287, 0.427592695},
        glm::vec3{0.473911583, -0.84311819, -0.25408566},
        glm::vec3{-0.156015098, -0.955536007, -0.250220358},
        glm::vec3{0.567212641, -0.7546525, 0.329802126},
        glm::vec3{0.551898658, -0.809262037, 0.201253086},
};

const auto badly_behaved_rays = util::aligned::vector<geo::ray>{
        geo::ray{glm::vec3{2.91376591, 2.99994421, 6},
                 glm::vec3{0.280182242, 0.849850773, -0.446375906}},
        geo::ray{glm::vec3{4, 2.65516424, 5.99998713},
                 glm::vec3{-0.501335502, -0.224737942, 0.835557043}},
        geo::ray{glm::vec3{0.00000566244125, 3, 0.800823509},
                 glm::vec3{-0.148433656, -0.987669587, 0.0497597083}},
        geo::ray{glm::vec3{0, 0.000000596046448, 1.53889632},
                 glm::vec3{0.434765905, -0.869531512, 0.234293744}}};

util::aligned::vector<geo::ray> get_random_rays(const glm::vec3& source, size_t num) {
    const auto directions = get_random_directions(num);
    return raytracer::get_rays_from_directions(
            directions.begin(), directions.end(), source);
}

template <typename Scene>
auto get_voxelised(Scene scene) {
    return make_voxelised_scene_data(scene, 5, 0.1f);
}

struct reflector_fixture : public ::testing::Test {
    const geo::box box{glm::vec3{0}, glm::vec3{4, 3, 6}};
    const voxelised_scene_data<cl_float3, surface<simulation_bands>> voxelised{
            get_voxelised(geo::get_scene_data(
                    box, make_surface<simulation_bands>(0, 0)))};
    const compute_context cc{};
    const scene_buffers buffers{cc.context, voxelised};

    const glm::vec3 source{1, 2, 1};
    const glm::vec3 receiver{2, 1, 2};

#define OPT (0)
#if OPT == 0
    const util::aligned::vector<geo::ray> rays{get_random_rays(source, 10000)};
#elif OPT == 1
    const util::aligned::vector<geo::ray> rays{raytracer::get_rays_from_directions(
            source, badly_behaved_directions)};
#elif OPT == 2
    const util::aligned::vector<geo::ray> rays{badly_behaved_rays};
#endif

    raytracer::reflector reflector{cc, receiver, begin(rays), end(rays)};

    auto get_fast_intersections() const {
        const auto rays = reflector.get_rays();
        const auto reflections = reflector.get_reflections();
        util::aligned::vector<std::experimental::optional<intersection>> ret;
        ret.reserve(rays.size());
        for (auto i = 0u; i != rays.size(); ++i) {
            ret.emplace_back(intersects(
                    voxelised, convert(rays[i]), reflections[i].triangle));
        }
        return ret;
    }

    auto get_slow_intersections() const {
        const auto rays = reflector.get_rays();
        const auto reflections = reflector.get_reflections();
        util::aligned::vector<std::experimental::optional<intersection>> ret;
        ret.reserve(rays.size());
        for (auto i = 0u; i != rays.size(); ++i) {
            ret.emplace_back(geo::ray_triangle_intersection(
                    convert(rays[i]),
                    voxelised.get_scene_data().get_triangles(),
                    convert(voxelised.get_scene_data().get_vertices()),
                    reflections[i].triangle));
        }
        return ret;
    }
};

TEST_F(reflector_fixture, locations) {
    const auto fast_intersections = get_fast_intersections();
    const auto slow_intersections = get_slow_intersections();
    for (auto i = 0u; i != fast_intersections.size(); ++i) {
        ASSERT_EQ(fast_intersections[i], slow_intersections[i]);
    }

    const auto current_rays = reflector.get_rays();
    const auto reflections = reflector.run_step(buffers);

    ASSERT_TRUE(std::any_of(begin(reflections),
                            end(reflections),
                            [](const auto& i) { return i.keep_going; }));
    ASSERT_TRUE(std::all_of(begin(reflections),
                            end(reflections),
                            [](const auto& i) { return i.keep_going; }));

    for (auto i = 0; i != current_rays.size(); ++i) {
        ASSERT_TRUE(fast_intersections[i]);
        const auto converted = convert(current_rays[i]);
        const auto cpu_position =
                converted.get_position() +
                (converted.get_direction() * fast_intersections[i]->inter.t);
        const auto gpu_position = to_vec3(reflections[i].position);
        ASSERT_TRUE(nearby(gpu_position, cpu_position, 0.00001));
    }
}

TEST_F(reflector_fixture, multi_layer_reflections) {
    for (auto i = 0u; i != 10; ++i) {
        const auto prev_reflections = reflector.get_reflections();
        const auto current_rays = reflector.get_rays();
        const auto fast_intersections = get_fast_intersections();
        const auto slow_intersections = get_slow_intersections();
        for (auto i = 0u; i != fast_intersections.size(); ++i) {
            ASSERT_EQ(fast_intersections[i], slow_intersections[i]);
        }
        const auto reflections = reflector.run_step(buffers);

        for (auto j = 0u; j != current_rays.size(); ++j) {
            ASSERT_TRUE(reflections[j].keep_going);
            ASSERT_TRUE(fast_intersections[j]);
            const auto converted = convert(current_rays[j]);
            const auto cpu_position =
                    converted.get_position() + (converted.get_direction() *
                                                fast_intersections[j]->inter.t);
            const auto gpu_position = to_vec3(reflections[j].position);
            const auto is_nearby = nearby(gpu_position, cpu_position, 0.00001);
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
