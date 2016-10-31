#include "frequency_domain/multiband_filter.h"

#include "utilities/foldl.h"

#include "gtest/gtest.h"

#include <random>

TEST(multiband, noise) {
    auto engine = std::default_random_engine{std::random_device{}()};
    auto dist = std::uniform_real_distribution<float>{-1, 1};

    const auto noise_signal = [&] {
        aligned::vector<float> ret;
        for (auto i = 0ul; i != 10000; ++i) {
            ret.emplace_back(dist(engine));
        }
        return ret;
    }();

    constexpr auto audible_range = range<double>{20, 20000};
    constexpr auto bands = 8;
    constexpr auto sample_rate = 44100.0;

    const auto range = audible_range / sample_rate;
    const auto params =
            frequency_domain::compute_multiband_params<bands>(range, 1);
    const auto energy = frequency_domain::per_band_energy(
            begin(noise_signal), end(noise_signal), params);

    const auto mean = foldl(std::plus<>{}, energy) / bands;

    for (auto i : energy) {
        ASSERT_NEAR(std::abs(mean - i) / mean, 0.0, 0.2);
    }
}
