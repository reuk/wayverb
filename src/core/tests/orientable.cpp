#include "core/orientation.h"
#include "core/az_el.h"

#include "gtest/gtest.h"

#include "glm/gtc/random.hpp"
#include "glm/gtx/rotate_vector.hpp"

using namespace wayverb::core;

TEST(orientation, compute_azimuth) {
    const auto test = [](auto pt, auto az) {
        ASSERT_NEAR(compute_azimuth(pt), az, 0.0001);
    };
    test(glm::vec3{-1, 0, 0}, -M_PI * 0.5);
    test(glm::vec3{1, 0, 0}, M_PI * 0.5);
    test(glm::vec3{0, -1, 0}, 0);
    test(glm::vec3{0, 1, 0}, 0);
    test(glm::vec3{0, 0, -1}, M_PI);
    test(glm::vec3{0, 0, 1}, 0);
}

TEST(orientation, compute_elevation) {
    const auto test = [](auto pt, auto el) {
        ASSERT_NEAR(compute_elevation(pt), el, 0.0001);
    };
    test(glm::vec3{-1, 0, 0}, 0);
    test(glm::vec3{1, 0, 0}, 0);
    test(glm::vec3{0, -1, 0}, -M_PI / 2);
    test(glm::vec3{0, 1, 0}, M_PI / 2);
    test(glm::vec3{0, 0, -1}, 0);
    test(glm::vec3{0, 0, 1}, 0);
}

TEST(orientation, round_trip) {
    const auto test = [](float az, float el) {
        const auto ret =
                compute_azimuth_elevation(compute_pointing(az_el{az, el}));
        ASSERT_NEAR(ret.azimuth, az, 0.0001) << az;
        ASSERT_NEAR(ret.elevation, el, 0.0001) << el;
    };

    test(0.0, 0.0);
    test(-M_PI * 0.5, 0.0);
    test(M_PI * 0.5, 0.0);
    test(0.0, -M_PI * 0.49);
    test(0.0, M_PI * 0.49);
}

namespace {
void near_vectors(const glm::vec3& a, const glm::vec3& b) {
    ASSERT_NEAR(glm::distance(a, b), 0, 0.00001);
}
}//namespace

TEST(orientation, pointing) {
    for (auto i = 0; i != 100; ++i) {
        const auto v = glm::sphericalRand(1.0f);

        near_vectors(transform(orientation{v}, v), glm::vec3{0, 0, 1});
    }
}
