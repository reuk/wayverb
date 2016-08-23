#include "common/nan_checking.h"
#include "common/string_builder.h"

#include "gtest/gtest.h"

#include <cmath>

float brdf_mag(float y, float d) {
    //  check that this direction is attainable
    if (y * y <= (1 - 2 * d) / pow(1 - d, 2)) {
        return 0;
    }

    const float a = pow(1 - d, 2) * pow(y, 2);
    const float b = (2 * d) - 1;
    const float numerator = (2 * a) + b;
    const float denominator = 2 * M_PI * d * sqrt(a + b);
    if (d < 0.5) {
        return numerator / denominator;
    }
    const float extra = ((1 - d) * y) / (2 * M_PI * d);
    return (numerator / (2 * denominator)) + extra;
}

TEST(brdf, brdf) {
    for (auto y{0.0}; y <= 1.0; y += 0.01) {
        for (auto d{0.0}; d <= 1.0; d += 0.01) {
            throw_if_suspicious(brdf_mag(y, d));
        }
    }
}
