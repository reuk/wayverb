#include "core/geo/box.h"
#include "core/geo/geometric.h"

#include "gtest/gtest.h"

using namespace wayverb::core;

TEST(geo_box, intersection_distances) {
    const geo::box box{glm::vec3{-1}, glm::vec3{1}};

    ASSERT_EQ(intersection_distances(
                      box, geo::ray{glm::vec3{0, 0, -2}, glm::vec3{0, 0, 1}}),
              std::experimental::make_optional(std::make_pair(1.0f, 3.0f)));
    ASSERT_EQ(intersection_distances(
                      box, geo::ray{glm::vec3{0, 0, 2}, glm::vec3{0, 0, 1}}),
              std::experimental::make_optional(std::make_pair(-3.0f, -1.0f)));
    ASSERT_EQ(intersection_distances(
                      box, geo::ray{glm::vec3{0, 0, 2}, glm::vec3{0, 1, 0}}),
              std::experimental::nullopt);
    ASSERT_EQ(intersection_distances(
                      box, geo::ray{glm::vec3{0, 0, 0}, glm::vec3{0, 0, 1}}),
              std::experimental::make_optional(std::make_pair(-1.0f, 1.0f)));
}

TEST(geo_box, intersects) {
    const geo::box box{glm::vec3{-1}, glm::vec3{1}};

    ASSERT_EQ(
            intersects(box, geo::ray{glm::vec3{0, 0, -2}, glm::vec3{0, 0, 1}}),
            std::experimental::make_optional(1.0f));
    ASSERT_EQ(intersects(box, geo::ray{glm::vec3{0, 0, 2}, glm::vec3{0, 0, 1}}),
              std::experimental::nullopt);

    ASSERT_TRUE(intersects(
            box, geo::ray{glm::vec3{0, 0, -2}, glm::vec3{0, 0, 1}}, 0, 10));

    ASSERT_FALSE(intersects(
            box, geo::ray{glm::vec3{0, 0, -2}, glm::vec3{0, 0, 1}}, 0, 1));

    ASSERT_FALSE(intersects(
            box, geo::ray{glm::vec3{0, 0, 2}, glm::vec3{0, 0, 1}}, 0, 10));
    ASSERT_TRUE(intersects(
            box,
            geo::ray{glm::vec3{-2, -2, -2}, glm::normalize(glm::vec3{1, 1, 1})},
            0,
            10));
}
