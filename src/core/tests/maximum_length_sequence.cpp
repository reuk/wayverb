#include "core/maximum_length_sequence.h"

#include "gtest/gtest.h"

#include <array>

TEST(mls, order_3) {
    const std::array<bool, 7> seq{{1, 0, 1, 1, 1, 0, 0}};

    core::generate_maximum_length_sequence<uint32_t>(
            3, [&](auto value, auto step) {
                ASSERT_EQ(value, seq[step]) << step;
            });
}

TEST(mls, order_4) {
    const std::array<bool, 15> seq{
            {1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0}};

    core::generate_maximum_length_sequence<uint32_t>(
            4, [&](auto value, auto step) {
                ASSERT_EQ(value, seq[step]) << step;
            });
}
