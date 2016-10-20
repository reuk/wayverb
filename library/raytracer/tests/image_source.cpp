#include "raytracer/image_source/exact.h"
#include "raytracer/image_source/get_direct.h"
#include "raytracer/image_source/run.h"
#include "raytracer/raytracer.h"

#include "gtest/gtest.h"

TEST(image_source, image_source_position) {
    const geo::box box{glm::vec3{0}, glm::vec3{4, 3, 6}};
    const glm::vec3 source{1};

    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{0, 0, 0}),
              source);

    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{1, 0, 0}),
              (glm::vec3{7, 1, 1}));
    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{-1, 0, 0}),
              (glm::vec3{-1, 1, 1}));

    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{0, 1, 0}),
              (glm::vec3{1, 5, 1}));
    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{0, -1, 0}),
              (glm::vec3{1, -1, 1}));

    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{0, 0, 1}),
              (glm::vec3{1, 1, 11}));
    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{0, 0, -1}),
              (glm::vec3{1, 1, -1}));

    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{2, 0, 0}),
              (glm::vec3{9, 1, 1}));
    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{-2, 0, 0}),
              (glm::vec3{-7, 1, 1}));

    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{0, 2, 0}),
              (glm::vec3{1, 7, 1}));
    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{0, -2, 0}),
              (glm::vec3{1, -5, 1}));

    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{0, 0, 2}),
              (glm::vec3{1, 1, 13}));
    ASSERT_EQ(raytracer::image_source::image_position(
                      box, source, glm::ivec3{0, 0, -2}),
              (glm::vec3{1, 1, -11}));
}

template <size_t channels>
bool approximately_matches(const impulse<channels>& a,
                           const impulse<channels>& b) {
    const auto near = [](auto a, auto b) { return nearby(a, b, 0.0001); };
    for (auto i = 0ul; i != channels; ++i) {
        if (!near (a.volume.s[i], b.volume.s[0])) {
            return false;
        }
    }
    return near(a.distance, b.distance) &&
           near(a.position.s[0], b.position.s[0]) &&
           near(a.position.s[1], b.position.s[1]) &&
           near(a.position.s[2], b.position.s[2]);
}

void image_source_test() {
    const geo::box box{glm::vec3{0, 0, 0}, glm::vec3{4, 3, 6}};
    constexpr model::parameters params{glm::vec3{1, 1, 1}, glm::vec3{2, 1, 5}};
    constexpr auto surface = make_surface<simulation_bands>(0.1f, 0);

    constexpr auto shells = 3;

    auto exact_impulses = raytracer::image_source::find_impulses(
            box, params.source, params.receiver, surface, shells, false);

    const auto check_distances = [&](const auto& range) {
        for (const auto& imp : range) {
            ASSERT_NEAR(glm::distance(to_vec3(params.receiver),
                                      to_vec3(imp.position)),
                        imp.distance,
                        0.0001);
        }
    };

    const auto check_volumes = [](const auto& range) {
        for (const auto& imp : range) {
            for (const auto& band : imp.volume.s) {
                ASSERT_LE(band, 1);
            }
        }
    };

    const auto check_range = [&](const auto& range) {
        check_distances(range);
        check_volumes(range);
    };

    check_range(exact_impulses);

    const auto voxelised = make_voxelised_scene_data(
            geo::get_scene_data(box, surface), 5, 0.1f);

    const auto directions = get_random_directions(10000);
    auto inexact_impulses = raytracer::image_source::run(directions.begin(),
                                                         directions.end(),
                                                         compute_context{},
                                                         voxelised,
                                                         params,
                                                         false);

    check_range(inexact_impulses);

    ASSERT_TRUE(inexact_impulses.size() > 1);

    const auto distance_comparator = [](const auto& a, const auto& b) {
        return a.distance < b.distance;
    };

    for (const auto& i : {&exact_impulses, &inexact_impulses}) {
        std::sort(i->begin(), i->end(), distance_comparator);
    }

    for (const auto& i : exact_impulses) {
        const auto possible_lower = std::lower_bound(inexact_impulses.begin(),
                                                     inexact_impulses.end(),
                                                     i,
                                                     distance_comparator);
        const auto lower = possible_lower == inexact_impulses.begin()
                                   ? possible_lower
                                   : possible_lower - 1;
        const auto possible_upper = std::upper_bound(inexact_impulses.begin(),
                                                     inexact_impulses.end(),
                                                     i,
                                                     distance_comparator);
        const auto upper = possible_upper == inexact_impulses.end()
                                   ? possible_upper
                                   : possible_upper + 1;
        aligned::vector<impulse<8>> possibilities(lower, upper);
        if (std::none_of(lower, upper, [&](const auto& x) {
                return approximately_matches(i, x);
            })) {
            throw std::runtime_error{"no approximate matches"};
        }
    }
}

TEST(image_source, fast_pressure) { ASSERT_NO_THROW(image_source_test()); }
