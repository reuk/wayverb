#include "common/dc_blocker.h"

#include "gtest/gtest.h"

TEST(dc_blocker, delay_line) {
    constexpr auto LENGTH = 4;

    filter::DelayLine dl(LENGTH);

    for (auto i = 0; i != LENGTH; ++i) {
        ASSERT_EQ(dl[i], 0);
    }

    dl.push(1);
    ASSERT_EQ(dl[0], 1);

    dl.push(2);
    ASSERT_EQ(dl[0], 2);
    ASSERT_EQ(dl[1], 1);

    dl.push(3);
    ASSERT_EQ(dl[0], 3);
    ASSERT_EQ(dl[1], 2);
    ASSERT_EQ(dl[2], 1);

    dl.push(0);
    dl.push(0);
    dl.push(0);
    dl.push(0);
    ASSERT_EQ(dl[0], 0);
    ASSERT_EQ(dl[1], 0);
    ASSERT_EQ(dl[2], 0);
}

TEST(dc_blocker, moving_average) {
    filter::MovingAverage ma(4);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);

    ASSERT_EQ(ma(1), 1 / 4.0);
    ASSERT_EQ(ma(0), 1 / 4.0);
    ASSERT_EQ(ma(0), 1 / 4.0);
    ASSERT_EQ(ma(0), 1 / 4.0);

    ASSERT_EQ(ma(0), 0);
    ASSERT_EQ(ma(0), 0);
}

TEST(dc_blocker, two_moving_average) {
    std::array<filter::MovingAverage, 2> ma{
        {filter::MovingAverage(4), filter::MovingAverage(4)}};

    auto apply = [&ma](double x) {
        for (auto& i : ma) {
            x = i(x);
        }
        return x;
    };

    ASSERT_EQ(apply(0), 0);
    ASSERT_EQ(apply(0), 0);
    ASSERT_EQ(apply(0), 0);
    ASSERT_EQ(apply(0), 0);
    ASSERT_EQ(apply(0), 0);
    ASSERT_EQ(apply(0), 0);

    ASSERT_EQ(apply(1), 1 / 16.0);
    ASSERT_EQ(apply(0), 2 / 16.0);
    ASSERT_EQ(apply(0), 3 / 16.0);
    ASSERT_EQ(apply(0), 4 / 16.0);

    ASSERT_EQ(apply(0), 3 / 16.0);
    ASSERT_EQ(apply(0), 2 / 16.0);
    ASSERT_EQ(apply(0), 1 / 16.0);
    ASSERT_EQ(apply(0), 0 / 16.0);

    ASSERT_EQ(apply(0), 0);
    ASSERT_EQ(apply(0), 0);
    ASSERT_EQ(apply(0), 0);
    ASSERT_EQ(apply(0), 0);
}

TEST(dc_blocker, dc_blocker) {
    filter::LinearDCBlocker dc(4);

    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);

    ASSERT_EQ(dc(1), -1 / 16.0);
    ASSERT_EQ(dc(0), -2 / 16.0);
    ASSERT_EQ(dc(0), -3 / 16.0);
    ASSERT_EQ(dc(0), 12 / 16.0);

    ASSERT_EQ(dc(0), -3 / 16.0);
    ASSERT_EQ(dc(0), -2 / 16.0);
    ASSERT_EQ(dc(0), -1 / 16.0);
    ASSERT_EQ(dc(0), -0 / 16.0);

    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
    ASSERT_EQ(dc(0), 0);
}

TEST(dc_blocker, big_offset) {
    filter::LinearDCBlocker dc(4);

    ASSERT_EQ(dc(2), -2 / 16.0);
    ASSERT_EQ(dc(2), -6 / 16.0);
    ASSERT_EQ(dc(2), -12 / 16.0);
    ASSERT_EQ(dc(2), 12 / 16.0);
    ASSERT_EQ(dc(2), 6 / 16.0);
    ASSERT_EQ(dc(2), 2 / 16.0);
}
