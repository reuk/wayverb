#include "core/schroeder.h"

#include "utilities/decibels.h"

#include "gtest/gtest.h"

#include <random>

using namespace wayverb::core;

TEST(schroeder, squared_integrated) {
    {
        util::aligned::vector<float> sig(100, 1.0f);
        const auto integrated = squared_integrated(sig.begin(), sig.end());
        ASSERT_EQ(integrated.back(), 100);
    }
    {
        util::aligned::vector<float> sig(100, -1.0f);
        const auto integrated = squared_integrated(sig.begin(), sig.end());
        ASSERT_EQ(integrated.back(), 100);
    }
    {
        util::aligned::vector<float> sig(10, 2.0f);
        const auto integrated = squared_integrated(sig.begin(), sig.end());
        ASSERT_EQ(integrated.back(), 40);
    }
}

auto generate_noise_tail(float rt60) {
    std::default_random_engine engine{std::random_device{}()};
    std::uniform_real_distribution<float> distribution{-1, 1};

    util::aligned::vector<float> ret;

    const auto min_amp = std::log(util::decibels::db2a(-60.0));
    for (auto i = 0u; i < rt60; ++i) {
        ret.emplace_back(distribution(engine) * std::exp(min_amp * i / rt60));
    }
    return ret;
}

TEST(schroeder, decay_time_from_points) {
    for (const auto& length : {1000.0, 2000.0, 10000.0, 20000.0, 100000.0}) {
        const auto noise = generate_noise_tail(length);

        ASSERT_NEAR(
                rt20(begin(noise), end(noise)).samples, length, length * 0.1);
        ASSERT_NEAR(
                rt30(begin(noise), end(noise)).samples, length, length * 0.1);
        ASSERT_NEAR(
                edt(begin(noise), end(noise)).samples, length, length * 0.1);
    }
}
