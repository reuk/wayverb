#include "raytracer/histogram.h"

#include "gtest/gtest.h"

struct item final {
    double volume;
    double time;
};

TEST(histogram, incremental) {
    constexpr auto sample_rate{1.0};
    constexpr auto max_time{100.0};

    const auto a = {item{1.0, 0.0}};
    const auto b = {item{1.0, 1.0}};
    const auto c = {item{1.0, 2.0}};
    aligned::vector<double> histogram;

    const auto add_to_histogram{[&](const auto& range) {
        raytracer::incremental_histogram(histogram,
                                         begin(range),
                                         end(range),
                                         sample_rate,
                                         max_time,
                                         raytracer::dirac_sum_functor{});
    }};

    add_to_histogram(a);
    add_to_histogram(b);
    add_to_histogram(c);

    ASSERT_EQ(histogram.size(), 3);
    ASSERT_EQ(histogram[0], 1.0);
    ASSERT_EQ(histogram[1], 1.0);
    ASSERT_EQ(histogram[2], 1.0);
}

TEST(histogram, dirac) {
    constexpr auto sample_rate{1.0};
    constexpr auto max_time{100.0};

    {
        const auto items = {item{1.0, 0.0}};
        const auto result{raytracer::histogram(items.begin(),
                                               items.end(),
                                               sample_rate,
                                               max_time,
                                               raytracer::dirac_sum_functor{})};
        ASSERT_EQ(result.size(), 1);
        ASSERT_EQ(result.front(), 1.0);
    }

    {
        const auto items = {item{2.0, 99.0}};
        const auto result{raytracer::histogram(items.begin(),
                                               items.end(),
                                               sample_rate,
                                               max_time,
                                               raytracer::dirac_sum_functor{})};
        ASSERT_EQ(result.size(), 100);
        ASSERT_EQ(result.back(), 2.0);
    }

    {
        const auto items = {item{0.0, 200.0}};
        const auto result{raytracer::histogram(items.begin(),
                                               items.end(),
                                               sample_rate,
                                               max_time,
                                               raytracer::dirac_sum_functor{})};
        ASSERT_EQ(result.size(), 101);
    }

    {
        const auto items = {item{1.0, 1.0}, item{2.0, 1.0}};
        const auto result{raytracer::histogram(items.begin(),
                                               items.end(),
                                               sample_rate,
                                               max_time,
                                               raytracer::dirac_sum_functor{})};
        ASSERT_EQ(result.size(), 2);
        ASSERT_EQ(result[1], 3.0);
    }
}

TEST(histogram, sinc) {
    constexpr auto sample_rate{1.0};
    constexpr auto max_time{1000.0};

    const auto sinc_callback{[](auto value, auto time, auto sr, auto& ret) {
        raytracer::sinc_sum(value, time, sr, ret);
    }};

    {
        const auto items = {item{1.0, 0.0}};
        const auto result{raytracer::histogram(items.begin(),
                                               items.end(),
                                               sample_rate,
                                               max_time,
                                               sinc_callback)};

        ASSERT_EQ(result.size(), 200);
        ASSERT_EQ(result.front(), 1.0);
    }

    {
        const auto items = {item{1.0, 0.0}, item{0.0, 10000.0}};
        const auto result{raytracer::histogram(items.begin(),
                                               items.end(),
                                               sample_rate,
                                               max_time,
                                               sinc_callback)};

        ASSERT_EQ(result.size(), max_time + 1);
        ASSERT_EQ(result.front(), 1.0);
        for (auto i{result.begin() + 1}, end{result.end()}; i != end; ++i) {
            ASSERT_NEAR(*i, 0.0, 0.0000001);
        }
    }
}
