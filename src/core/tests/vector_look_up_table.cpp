#include "core/vector_look_up_table.h"

#include "gtest/gtest.h"

using namespace wayverb::core;

namespace {

using test_table_a = vector_look_up_table<float, 24, 11>;

static_assert(test_table_a::get_azimuth_angle() == 15, "get_azimuth_angle");
static_assert(test_table_a::get_elevation_angle() == 15, "get_elevation_angle");

static_assert(test_table_a::azimuth_to_index(0) == 0, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(5) == 0, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(-5) == 0, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(355) == 0, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(10) == 1, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(15) == 1, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(20) == 1, "azimuth_to_index");
static_assert(test_table_a::azimuth_to_index(-45) == 21, "azimuth_to_index");

static_assert(test_table_a::elevation_to_index(0) == 5, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(5) == 5, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(-5) == 5, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(355) == 5, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(10) == 6, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(15) == 6, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(20) == 6, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(90) == 10, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(-90) == 0, "elevation_to_index");
static_assert(test_table_a::elevation_to_index(270) == 0, "elevation_to_index");

using test_table_b = vector_look_up_table<float, 20, 9>;

static_assert(test_table_b::get_azimuth_angle() == 18, "get_azimuth_angle");
static_assert(test_table_b::get_elevation_angle() == 18, "get_elevation_angle");

static_assert(test_table_b::azimuth_to_index(0) == 0, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(8) == 0, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(9) == 1, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(26) == 1, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(27) == 2, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(355) == 0, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(-5) == 0, "azimuth_to_index");
static_assert(test_table_b::azimuth_to_index(180) == 10, "azimuth_to_index");

static_assert(test_table_b::elevation_to_index(0) == 4, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(-8) == 4, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(8) == 4, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(9) == 5, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(-10) == 3, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(90) == 8, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(270) == 0, "elevation_to_index");
static_assert(test_table_b::elevation_to_index(-90) == 0, "elevation_to_index");

}  // namespace

TEST(vector_look_up_table, index) {
    using table = vector_look_up_table<float, 20, 9>;

    const auto test = [](auto pt, auto azel) {
        const auto result = table::index(pt);
        ASSERT_EQ(result.azimuth, azel.azimuth);
        ASSERT_EQ(result.elevation, azel.elevation);
    };
    test(glm::vec3{0, 0, 1}, table::index_pair{0, 4});
    test(glm::vec3{0, 0, -1}, table::index_pair{10, 4});

    test(glm::vec3{0, 1, 0}, table::index_pair{0, 8});
    test(glm::vec3{0, -1, 0}, table::index_pair{0, 0});

    test(glm::vec3{1, 0, 0}, table::index_pair{15, 4});
    test(glm::vec3{-1, 0, 0}, table::index_pair{5, 4});
}

TEST(vector_look_up_table, pointing) {
    using table = vector_look_up_table<float, 20, 9>;

    const auto test = [](auto azel, auto pt) {
        const auto results = table::pointing(azel);
        ASSERT_NEAR(glm::distance(results, pt), 0, 0.000001);
    };

    test(table::index_pair{0, 4}, glm::vec3{0, 0, 1});
    test(table::index_pair{10, 4}, glm::vec3{0, 0, -1});

    {
        const auto angle = M_PI * 2 / 5;
        test(table::index_pair{0, 8},
             glm::vec3{0, std::sin(angle), std::cos(angle)});
    }

    {
        const auto angle = -M_PI * 2 / 5;
        test(table::index_pair{0, 0},
             glm::vec3{0, std::sin(angle), std::cos(angle)});
    }

    test(table::index_pair{15, 4}, glm::vec3{1, 0, 0});
    test(table::index_pair{5, 4}, glm::vec3{-1, 0, 0});
}
