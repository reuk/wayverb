#include "frequency_domain/multiband_filter.h"

#include "utilities/reduce.h"

#include "gtest/gtest.h"

#include <random>

TEST(multiband, noise) {
    auto engine = std::default_random_engine{std::random_device{}()};
    auto dist = std::uniform_real_distribution<float>{-1, 1};

    const auto noise_signal = [&] {
        aligned::vector<float> ret;
        for (auto i = 0ul; i != 100000; ++i) {
            ret.emplace_back(dist(engine));
        }
        return ret;
    }();

    constexpr auto audible_range = range<double>{20, 20000};
    constexpr auto bands = 8;
    constexpr auto sample_rate = 44100.0;

    const auto params = frequency_domain::band_edges_and_widths<bands>(
            audible_range / sample_rate, 1);

    const auto energy = frequency_domain::per_band_energy(
            begin(noise_signal), end(noise_signal), params);

    const auto mean = reduce(std::plus<>{}, energy) / bands;

    ASSERT_NEAR(mean, 1.0, 0.01);

    for (auto i : energy) {
        ASSERT_NEAR(i, mean, 0.1);
    }
}
