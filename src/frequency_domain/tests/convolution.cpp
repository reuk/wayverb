#include "frequency_domain/convolver.h"

#include "gtest/gtest.h"

TEST(convolution, convolution) {
    std::vector<float> a{1, 0, 0, 0, 0};
    std::vector<float> b{1, 2, 3, 4, 3, 2, 1, 0, 0};

    frequency_domain::convolver fc{a.size() + b.size() - 1};
    const auto convolved{fc.convolve(a, b)};

    const auto desired{std::vector<float>{1, 2, 3, 4, 3, 2, 1}};
    for (auto i{0u}; i != desired.size(); ++i) {
        ASSERT_NEAR(convolved[i], desired[i], 0.0001);
    }
}
