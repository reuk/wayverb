#include "common/orientable.h"

#include "gtest/gtest.h"

TEST(orientable, compute_azimuth) {
    const auto test{[](auto pt, auto az) {
        ASSERT_NEAR(compute_azimuth(pt), az, 0.0001);
    }};
    test(glm::vec3{-1, 0, 0}, -M_PI * 0.5);
    test(glm::vec3{1, 0, 0}, M_PI * 0.5);
    test(glm::vec3{0, -1, 0}, 0);
    test(glm::vec3{0, 1, 0}, 0);
    test(glm::vec3{0, 0, -1}, M_PI);
    test(glm::vec3{0, 0, 1}, 0);
}

TEST(orientable, compute_elevation) {
    const auto test{[](auto pt, auto el) {
        ASSERT_NEAR(compute_elevation(pt), el, 0.0001);
    }};
    test(glm::vec3{-1, 0, 0}, 0);
    test(glm::vec3{1, 0, 0}, 0);
    test(glm::vec3{0, -1, 0}, -M_PI / 2);
    test(glm::vec3{0, 1, 0}, M_PI / 2);
    test(glm::vec3{0, 0, -1}, 0);
    test(glm::vec3{0, 0, 1}, 0);
}

TEST(orientable, round_trip) {
    const auto test{[](float az, float el) {
        const auto ret{compute_azimuth_elevation(compute_pointing(az_el{az, el}))};
        ASSERT_NEAR(ret.azimuth, az, 0.0001) << az;
        ASSERT_NEAR(ret.elevation, el, 0.0001) << el;
    }};

    test(0.0, 0.0);
    test(-M_PI * 0.5, 0.0);
    test(M_PI * 0.5, 0.0);
    test(0.0, -M_PI * 0.49);
    test(0.0, M_PI * 0.49);
}
