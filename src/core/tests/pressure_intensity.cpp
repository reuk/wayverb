#include "core/pressure_intensity.h"

#include "gtest/gtest.h"

namespace {
const auto acoustic_impedance{400.0};
}  // namespace

void test_distance(double distance) {
    const auto intensity{intensity_for_distance(distance)};
    const auto pressure{pressure_for_distance(distance, acoustic_impedance)};
    ASSERT_NEAR(intensity_to_pressure(intensity, acoustic_impedance),
                pressure,
                0.000001);
    ASSERT_NEAR(pressure_to_intensity(pressure, acoustic_impedance),
                intensity,
                0.000001);
}

TEST(pressure_intensity, conversions) {
    test_distance(1 << 1);
    test_distance(1 << 2);
    test_distance(1 << 3);
    test_distance(1 << 4);
    test_distance(1 << 5);
}
