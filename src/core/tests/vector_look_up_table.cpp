#include "core/vector_look_up_table.h"

#include "gtest/gtest.h"

using namespace wayverb::core;

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
