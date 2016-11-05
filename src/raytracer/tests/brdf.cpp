#include "core/nan_checking.h"

#include "utilities/string_builder.h"

#include "gtest/gtest.h"

#include <cmath>
#include <random>

using namespace wayverb::core;

namespace {
float get_frac(float numerator, float denominator) {
    return denominator != 0.0f ? numerator / denominator : 0;
}

float brdf_mag(float y, float d) {
    const float y_sq = y * y;
    const float one_minus_d_sq = pow(1 - d, 2);
    const float numerator = 2 * one_minus_d_sq * y_sq + 2 * d - 1;

    const float to_sqrt = std::max(one_minus_d_sq * y_sq + 2 * d - 1, 0.0f);

    if (0.5 <= d) {
        const float denominator = 4 * M_PI * d * sqrt(to_sqrt);
        const float extra = ((1 - d) * y) / (2 * M_PI * d);
        return get_frac(numerator, denominator) + extra;
    }

    const float denominator = 2 * M_PI * d * sqrt(to_sqrt);
    return get_frac(numerator, denominator);
}

auto check_inf(float y, float d) {
    const auto brdf = brdf_mag(y, d);
    if (is_any_inf(brdf)) {
        std::cout << util::build_string("inf! y: ", y, ", d: ", d, '\n');
    }
}

auto check_nan(float y, float d) {
    const auto brdf = brdf_mag(y, d);
    if (is_any_nan(brdf)) {
        std::cout << util::build_string("nan! y: ", y, ", d: ", d, '\n');
    }
}

TEST(brdf, brdf) {
    std::default_random_engine engine{std::random_device{}()};
    std::uniform_real_distribution<float> distribution{0, 1};

    for (auto i = 0u; i != 1 << 21; ++i) {
        //  edge cases
        const auto a = distribution(engine);
        const auto b = distribution(engine);

        for (auto j : std::vector<float>{0, 0.5, 1}) {
            check_inf(a, j);
            check_inf(b, j);

            check_nan(a, j);
            check_nan(b, j);
        }

        check_inf(a, b);
        check_inf(b, a);

        check_nan(a, b);
        check_nan(b, a);
    }
}

}  // namespace
