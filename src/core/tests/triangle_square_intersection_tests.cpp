#include "core/geo/overlaps_2d.h"

#include "gtest/gtest.h"

#include <array>
#include <map>
#include <set>
#include <vector>

TEST(triangle_square_intersection, perp) {
    ASSERT_EQ(geo::detail::perp(glm::vec2(0, 1)), glm::vec2(-1, 0));
    ASSERT_EQ(geo::detail::perp(glm::vec2(2, 2)), glm::vec2(-2, 2));
    ASSERT_EQ(geo::detail::perp(glm::vec2(-4, 2)), glm::vec2(-2, -4));
}

TEST(triangle_square_intersection, normal_2d) {
    ASSERT_EQ(geo::detail::normal_2d(glm::vec2(0, 0), glm::vec2(0, 1)),
              glm::vec2(-1, 0));
    ASSERT_EQ(geo::detail::normal_2d(glm::vec2(0, 0), glm::vec2(2, 2)),
              glm::vec2(-2, 2));
    ASSERT_EQ(geo::detail::normal_2d(glm::vec2(0, 0), glm::vec2(-4, 2)),
              glm::vec2(-2, -4));
}

TEST(triangle_square_intersection, normals_2d) {
    std::array<glm::vec2, 3> to_test{
            {glm::vec2(0, 1), glm::vec2(2, 2), glm::vec2(-4, 2)}};
    std::array<glm::vec2, 3> out;
    geo::detail::normals_2d(to_test.begin(), to_test.end(), out.begin());
    ASSERT_EQ(out,
              (std::array<glm::vec2, 3>{
                      {glm::vec2(1, 4), glm::vec2(-1, 2), glm::vec2(0, -6)}}));
}

TEST(triangle_square_intersection, project) {
    {
        std::array<glm::vec2, 1> to_test{{glm::vec2(2, 5)}};
        ASSERT_EQ(geo::detail::project(
                          to_test.begin(), to_test.end(), glm::vec2(1, 0)),
                  (geo::detail::projection_2d{2, 2}));
        ASSERT_EQ(geo::detail::project(
                          to_test.begin(), to_test.end(), glm::vec2(-1, 0)),
                  (geo::detail::projection_2d{-2, -2}));
        ASSERT_EQ(geo::detail::project(
                          to_test.begin(), to_test.end(), glm::vec2(0, 1)),
                  (geo::detail::projection_2d{5, 5}));
        ASSERT_EQ(geo::detail::project(
                          to_test.begin(), to_test.end(), glm::vec2(0, -1)),
                  (geo::detail::projection_2d{-5, -5}));
    }

    {
        std::array<glm::vec2, 2> to_test{{glm::vec2(2, 5), glm::vec2(-4, 1)}};
        ASSERT_EQ(geo::detail::project(
                          to_test.begin(), to_test.end(), glm::vec2(1, 0)),
                  (geo::detail::projection_2d{-4, 2}));
        ASSERT_EQ(geo::detail::project(
                          to_test.begin(), to_test.end(), glm::vec2(-1, 0)),
                  (geo::detail::projection_2d{-2, 4}));
        ASSERT_EQ(geo::detail::project(
                          to_test.begin(), to_test.end(), glm::vec2(0, 1)),
                  (geo::detail::projection_2d{1, 5}));
        ASSERT_EQ(geo::detail::project(
                          to_test.begin(), to_test.end(), glm::vec2(0, -1)),
                  (geo::detail::projection_2d{-5, -1}));
    }
}

TEST(triangle_square_intersection, overlap) {
    std::array<glm::vec2, 2> a{{glm::vec2(0, 0), glm::vec2(1, 0)}};
    std::array<glm::vec2, 2> b{{glm::vec2(0, 1), glm::vec2(1, 1)}};
    ASSERT_TRUE(geo::detail::overlaps(
            a.begin(), a.end(), b.begin(), b.end(), glm::vec2(1, 0)));
    ASSERT_TRUE(geo::detail::overlaps(
            a.begin(), a.end(), b.begin(), b.end(), glm::vec2(-1, 0)));
    ASSERT_FALSE(geo::detail::overlaps(
            a.begin(), a.end(), b.begin(), b.end(), glm::vec2(0, 1)));
    ASSERT_FALSE(geo::detail::overlaps(
            a.begin(), a.end(), b.begin(), b.end(), glm::vec2(0, -1)));

    std::array<glm::vec2, 2> i{{glm::vec2(1, 0), glm::vec2(-1, 0)}};
    std::array<glm::vec2, 2> j{{glm::vec2(0, 1), glm::vec2(0, -1)}};
    ASSERT_TRUE(geo::detail::overlaps(
            a.begin(), a.end(), b.begin(), b.end(), i.begin(), i.end()));
    ASSERT_FALSE(geo::detail::overlaps(
            a.begin(), a.end(), b.begin(), b.end(), j.begin(), j.end()));
}

TEST(triangle_square_intersection, overlaps_2d) {
    ASSERT_FALSE(geo::overlaps_2d(
            std::array<glm::vec2, 2>{{glm::vec2(0, 0), glm::vec2(1, 0)}},
            std::array<glm::vec2, 2>{{glm::vec2(0, 1), glm::vec2(1, 1)}}));

    ASSERT_TRUE(geo::overlaps_2d(
            std::array<glm::vec2, 3>{
                    {glm::vec2(0, 0), glm::vec2(2, 0), glm::vec2(0, 2)}},
            std::array<glm::vec2, 2>{{glm::vec2(-1, 1), glm::vec2(1, 1)}}));
}
