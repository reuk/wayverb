#include "common/freqz.h"

#include "gtest/gtest.h"

TEST(freqz, flat) {
    ASSERT_NEAR(std::abs(freqz(std::array<double, 2>{{1, 0}},
                               std::array<double, 3>{{1, 0, 0}},
                               0.25)),
                1.0,
                0.00000001);

    ASSERT_NEAR(std::abs(freqz(std::array<double, 2>{{0.2, 0}},
                               std::array<double, 3>{{1, 0, 0}},
                               0.9)),
                0.2,
                0.00000001);
}
