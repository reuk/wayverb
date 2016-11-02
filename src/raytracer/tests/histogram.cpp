#include "raytracer/histogram.h"

#include "gtest/gtest.h"

using namespace wayverb::raytracer;
using namespace wayverb::core;

namespace {

struct item final {
    double volume;
    double time;
};

TEST(histogram, incremental) {
    constexpr auto sample_rate=1.0;

    const auto a = {item{1.0, 0.0}};
    const auto b = {item{1.0, 1.0}};
    const auto c = {item{1.0, 2.0}};
    util::aligned::vector<double> histogram;

    const auto add_to_histogram = [&](const auto& range) {
        incremental_histogram(histogram,
                                         begin(range),
                                         end(range),
                                         sample_rate,
                                         dirac_sum_functor{});
    };

    add_to_histogram(a);
    add_to_histogram(b);
    add_to_histogram(c);

    ASSERT_EQ(histogram.size(), 3);
    ASSERT_EQ(histogram[0], 1.0);
    ASSERT_EQ(histogram[1], 1.0);
    ASSERT_EQ(histogram[2], 1.0);
}

TEST(histogram, dirac) {
    constexpr auto sample_rate=1.0;

    {
        const auto items = {item{1.0, 0.0}};
        const auto result = histogram(
                items.begin(), items.end(), sample_rate, dirac_sum_functor{});
        ASSERT_EQ(result.size(), 1);
        ASSERT_EQ(result.front(), 1.0);
    }

    {
        const auto items = {item{2.0, 99.0}};
        const auto result = histogram(
                items.begin(), items.end(), sample_rate, dirac_sum_functor{});
        ASSERT_EQ(result.size(), 100);
        ASSERT_EQ(result.back(), 2.0);
    }

    {
        const auto items = {item{1.0, 1.0}, item{2.0, 1.0}};
        const auto result = histogram(
                items.begin(), items.end(), sample_rate, dirac_sum_functor{});
        ASSERT_EQ(result.size(), 2);
        ASSERT_EQ(result[1], 3.0);
    }
}

TEST(histogram, sinc) {
    constexpr auto sample_rate = 1.0;

    {
        const auto items = {item{1.0, 0.0}};
        const auto result = histogram(
                items.begin(), items.end(), sample_rate, sinc_sum_functor{});

        ASSERT_EQ(result.size(), 200);
        ASSERT_EQ(result.front(), 1.0);
    }
}
}  // namespace
