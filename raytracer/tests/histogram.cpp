#include "raytracer/histogram.h"

#include "common/write_audio_file.h"

#include "gtest/gtest.h"

struct item final {
    double volume;
    double time;
};

TEST(histogram, dirac) {
    constexpr auto sample_rate{1.0};
    constexpr auto max_time{100.0};

    {
        const auto items = {item{1.0, 0.0}};
        const auto result{raytracer::dirac_histogram(
                items.begin(), items.end(), sample_rate, max_time)};
        ASSERT_EQ(result.size(), 1);
        ASSERT_EQ(result.front(), 1.0);
    }

    {
        const auto items = {item{2.0, 99.0}};
        const auto result{raytracer::dirac_histogram(
                items.begin(), items.end(), sample_rate, max_time)};
        ASSERT_EQ(result.size(), 100);
        ASSERT_EQ(result.back(), 2.0);
    }

    {
        const auto items = {item{0.0, 200.0}};
        const auto result{raytracer::dirac_histogram(
                items.begin(), items.end(), sample_rate, max_time)};
        ASSERT_EQ(result.size(), 101);
    }

    {
        const auto items = {item{1.0, 1.0}, item{2.0, 1.0}};
        const auto result{raytracer::dirac_histogram(
                items.begin(), items.end(), sample_rate, max_time)};
        ASSERT_EQ(result.size(), 2);
        ASSERT_EQ(result[1], 3.0);
    }
}

TEST(histogram, sinc) {
    constexpr auto sample_rate{1.0};
    constexpr auto max_time{1000.0};

    {
        const auto items = {item{1.0, 0.0}};
        const auto result{raytracer::sinc_histogram(
                items.begin(), items.end(), sample_rate, max_time)};

        ASSERT_EQ(result.size(), 1.0);
        ASSERT_EQ(result.front(), 1.0);
    }

    {
        const auto items = {item{1.0, 0.0}, item{0.0, 10000.0}};
        const auto result{raytracer::sinc_histogram(
                items.begin(), items.end(), sample_rate, max_time)};

        ASSERT_EQ(result.size(), max_time + 1);
        ASSERT_EQ(result.front(), 1.0);
        for (auto i{result.begin() + 1}, end{result.end()}; i != end; ++i) {
            ASSERT_EQ(*i, 0.0);
        }
    }
}
