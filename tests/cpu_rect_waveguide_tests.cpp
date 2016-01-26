#include "cpu_rect_waveguide.h"

#include "gtest/gtest.h"

TEST(cpu_rect_waveguide, construct) {
    auto bounds = Vec3f(10, 10, 10);
    auto cb = CuboidBoundary(-bounds, bounds);
    auto waveguide = CpuRectangularWaveguide(cb, 0.1, Vec3f(0));
}
