#include "raytracer/image_source/exact.h"
#include "raytracer/postprocess.h"
#include "raytracer/raytracer.h"
#include "raytracer/reflector.h"

#include "common/azimuth_elevation.h"
#include "common/cl/common.h"
#include "common/conversions.h"
#include "common/dsp_vector_ops.h"
#include "common/map_to_vector.h"
#include "common/scene_data_loader.h"
#include "common/spatial_division/voxelised_scene_data.h"
#include "common/string_builder.h"

#include "gtest/gtest.h"

#include <set>

#ifndef OBJ_PATH
#define OBJ_PATH ""
#endif

#ifndef SCRATCH_PATH
#define SCRATCH_PATH ""
#endif

constexpr auto bench_reflections{128};
constexpr auto bench_rays{1 << 15};

const glm::vec3 source{1, 2, 1};
const auto the_rays = get_random_directions(bench_rays);

TEST(raytrace, new) {
    const compute_context cc{};

    const auto scene{scene_with_extracted_surfaces(
            scene_data_loader{OBJ_PATH}.get_scene_data())};
    const auto voxelised{make_voxelised_scene_data(scene, 5, 0.1f)};

    auto results{raytracer::run(cc,
                                voxelised,
                                source,
                                glm::vec3(0, 1.75, 0),
                                the_rays,
                                bench_reflections,
                                10,
                                true,
                                [](auto) {})};

    ASSERT_TRUE(results);
}

TEST(raytrace, same_location) {
    geo::box box{glm::vec3{0, 0, 0}, glm::vec3{4, 3, 6}};
    auto receiver{source};
    constexpr auto s{0.1};
    constexpr auto d{0.1};
    constexpr auto surface{make_surface(s, d)};

    const auto scene{geo::get_scene_data(box, surface)};
    const auto voxelised{make_voxelised_scene_data(scene, 5, 0.1f)};

    const compute_context cc;

    auto callback_count{0};

    std::atomic_bool keep_going{true};
    const auto results =
            raytracer::run(cc,
                           voxelised,
                           source,
                           receiver,
                           the_rays,
                           bench_reflections,
                           10,
                           keep_going,
                           [&](auto i) { ASSERT_EQ(i, callback_count++); });

    ASSERT_TRUE(results);

    const auto diffuse = results->get_diffuse();

    for (auto i = 0u; i != bench_rays; ++i) {
        const auto intersection =
                intersects(voxelised, geo::ray{source, the_rays[i]});
        if (intersection) {
            const auto cpu_position =
                    source + (the_rays[i] * intersection->inter.t);
            const auto gpu_position = to_vec3(diffuse[i][0].position);
            if (!nearby(cpu_position, gpu_position, 0.0001)) {
                ;
            }
        }
    }
}
