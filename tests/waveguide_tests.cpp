#include "waveguide.h"

#include "gtest/gtest.h"

#include <random>

static constexpr float sr{44100};

TEST(notch_filter_coefficients, notch_filter_coefficients) {
    static std::default_random_engine engine{std::random_device()()};
    static std::uniform_real_distribution<cl_float> range{0, sr / 2};
    for (auto i = 0; i != 10; ++i) {
        auto descriptor =
            RectangularProgram::FilterDescriptor{0, range(engine), 1.414};
        auto coefficients =
            RectangularProgram::get_notch_coefficients(descriptor, sr);

        ASSERT_TRUE(std::equal(std::begin(coefficients.b),
                               std::end(coefficients.b),
                               std::begin(coefficients.a)));
    }
}

TEST(filter_coefficients, filter_coefficients) {
    auto coefficients = RectangularWaveguide::to_filter_coefficients(
        Surface{{{1, 1, 1, 1, 1, 1, 1, 1}}, {{1, 1, 1, 1, 1, 1, 1, 1}}}, sr);
    ASSERT_TRUE(std::equal(std::begin(coefficients.b),
                           std::end(coefficients.b),
                           std::begin(coefficients.a)));
}
