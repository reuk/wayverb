#include "core/geo/rect.h"

#include "gtest/gtest.h"

TEST(rect, overlaps) {
    core::geo::rect r(glm::vec2(0, 0), glm::vec2(1, 1));
    ASSERT_FALSE(core::geo::overlaps(
            r, std::array<glm::vec2, 2>{{glm::vec2(-1, 0), glm::vec2(-1, 1)}}));
    ASSERT_FALSE(core::geo::overlaps(
            r, std::array<glm::vec2, 2>{{glm::vec2(2, 0), glm::vec2(2, 1)}}));
    ASSERT_FALSE(core::geo::overlaps(
            r, std::array<glm::vec2, 2>{{glm::vec2(0, -1), glm::vec2(1, -1)}}));
    ASSERT_FALSE(core::geo::overlaps(
            r, std::array<glm::vec2, 2>{{glm::vec2(0, 2), glm::vec2(0, 2)}}));

    ASSERT_TRUE(core::geo::overlaps(
            r, std::array<glm::vec2, 2>{{glm::vec2(0, 0), glm::vec2(1, 1)}}));

    ASSERT_TRUE(core::geo::overlaps(
            r,
            std::array<glm::vec2, 3>{
                    {glm::vec2(0, -1), glm::vec2(1, -1), glm::vec2(1, 2)}}));
}
