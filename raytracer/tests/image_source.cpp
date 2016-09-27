#include "raytracer/image_source/exact.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "common/write_audio_file.h"

#include "gtest/gtest.h"

template <typename callback>
void image_source_test() {
    const geo::box box{glm::vec3{0, 0, 0}, glm::vec3{4, 3, 6}};
    constexpr glm::vec3 source{1, 1, 1};
    constexpr glm::vec3 receiver{2, 1, 5};
    constexpr auto absorption{0.1};
    constexpr auto surface{make_surface(absorption, 0)};

    constexpr auto speed_of_sound{340.0};

    auto exact_impulses{raytracer::image_source::find_impulses<callback>(
            box, source, receiver, surface, 3)};

    const auto voxelised{make_voxelised_scene_data(
            geo::get_scene_data(box, surface), 5, 0.1f)};

    const auto directions{get_random_directions(10000)};
    auto inexact_impulses{
            raytracer::image_source::run<callback>(directions.begin(),
                                                   directions.end(),
                                                   compute_context{},
                                                   voxelised,
                                                   source,
                                                   receiver,
                                                   speed_of_sound)};

    if (const auto direct{raytracer::get_direct(source, receiver, voxelised)}) {
        inexact_impulses.emplace_back(
                raytracer::image_source::generic_impulse<volume_type>{
                        direct->volume,
                        to_vec3(direct->position),
                        direct->distance});
    }

    ASSERT_TRUE(inexact_impulses.size() > 1);

    for (const auto& i : {&exact_impulses, &inexact_impulses}) {
        std::sort(i->begin(), i->end(), [](auto a, auto b) {
            return a.distance < b.distance;
        });
    }


    for (const auto& i : exact_impulses) {
        const auto closest{
                *std::min_element(inexact_impulses.begin(),
                                  inexact_impulses.end(),
                                  [&](const auto& a, const auto& b) {
                                      return std::abs(a.distance - i.distance) <
                                             std::abs(b.distance - i.distance);
                                  })};
        ASSERT_NEAR(i.distance, closest.distance, 0.0001);
        ASSERT_NEAR(i.volume.s[0], closest.volume.s[0], 0.0001);
        ASSERT_NEAR(i.position.x, closest.position.x, 0.0001);
        ASSERT_NEAR(i.position.y, closest.position.y, 0.0001);
        ASSERT_NEAR(i.position.z, closest.position.z, 0.0001);
    }
}

TEST(image_source, intensity) {
    image_source_test<raytracer::image_source::intensity_calculator<>>();
}

TEST(image_source, fast_pressure) {
    image_source_test<raytracer::image_source::fast_pressure_calculator<>>();
}
