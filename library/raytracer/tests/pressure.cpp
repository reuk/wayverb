#include "raytracer/pressure.h"

#include "utilities/aligned/vector.h"
#include "utilities/mapping_iterator_adapter.h"

#include "gtest/gtest.h"

using context = raytracer::img_src_calculation<8>;

TEST(pressure, absorption_to_reflection_factor) {
    ASSERT_EQ(context::absorption_to_reflection_factor(0), 1);
    ASSERT_EQ(context::absorption_to_reflection_factor(1), 0);
    ASSERT_EQ(context::absorption_to_reflection_factor(0.75), 0.5);
}

TEST(pressure, compute_phase) {
    ASSERT_EQ(context::compute_phase(0, 0), 0);
    ASSERT_EQ(context::compute_phase(100, 0), 0);
    ASSERT_EQ(context::compute_phase(0, 1), 0);
    ASSERT_NEAR(context::compute_phase(1, 0.5), M_PI, 0.0000001);
}

TEST(pressure, cosine_smoothing) {
    const context::bins<context::real> per_band_amplitudes{
            {0.1, 0.2, 0.1, 0.2, 0.1, 0.2, 0.1, 0.2}};
    const context::bins<context::real> per_band_frequencies{
            {50, 100, 200, 400, 800, 1600, 3200, 6400}};
    const auto sample_rate{44100.0};

    const auto smoothed{context::cosine_smoothing<1 << 10>(
            per_band_amplitudes, per_band_frequencies, sample_rate)};

    const auto minmax{std::minmax_element(smoothed.begin(), smoothed.end())};
    ASSERT_NEAR(*minmax.first, 0.0, 0.0001);
    ASSERT_NEAR(*minmax.second, 0.2, 0.0001);
}

TEST(pressure, compute_pressure_spectrum) {
    util::aligned::vector<context::bins<context::real>> absorption_coefficients{
            context::bins<context::real>{
                    {0.1, 0.1, 0.2, 0.2, 0.3, 0.3, 0.4, 0.4}},
            context::bins<context::real>{
                    {0.2, 0.2, 0.2, 0.3, 0.3, 0.3, 0.4, 0.5}},
            context::bins<context::real>{
                    {0.0, 0.0, 0.0, 0.0, 0.1, 0.2, 0.3, 0.4}}};

    util::aligned::vector<size_t> surface_indices{0, 0, 1, 0, 2};

    const auto coefficient_indexer{
            [&](auto i) { return absorption_coefficients[i]; }};

    const auto distance{10.0};
    const auto speed_of_sound{340.0};
    //const auto acoustic_impedance{400.0};

    context::bins<context::real> centre_frequencies{
            {50, 100, 200, 400, 800, 1600, 3200, 6400}};

    const auto pressure_spectrum{context::compute_pressure_spectrum(
            util::make_mapping_iterator_adapter(surface_indices.begin(),
                                                coefficient_indexer),
            util::make_mapping_iterator_adapter(surface_indices.end(),
                                                coefficient_indexer),
            distance,
            speed_of_sound,
            1.0,
            centre_frequencies)};

    for (auto i : pressure_spectrum) {
        std::cout << i << std::endl;
    }
}
