#include "core/almost_equal.h"
#include "core/az_el.h"
#include "core/orientation.h"
#include "utilities/string_builder.h"

#include "gtest/gtest.h"

#include "glm/gtc/random.hpp"
#include "glm/gtx/rotate_vector.hpp"

using namespace wayverb::core;

TEST(orientation, compute_azimuth) {
    const auto test = [](auto pt, auto az) {
        ASSERT_NEAR(compute_azimuth(pt), az, 0.0001);
    };

    test(normalize(glm::vec3{-1, 0, -1}), -M_PI / 4);
    test(normalize(glm::vec3{1, 0, -1}), M_PI / 4);

    test(glm::vec3{-1, 0, 0}, -M_PI / 2);
    test(glm::vec3{1, 0, 0}, M_PI / 2);

    test(glm::vec3{0, 0, -1}, 0);
    test(glm::vec3{0, 0, 1}, M_PI);
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

TEST(orientation, compute_pointing) {
    const auto eps = 0.00001;
    ASSERT_TRUE(
            nearby(compute_pointing(az_el{0, 0}), glm::vec3{0, 0, -1}, eps));
    ASSERT_TRUE(nearby(
            compute_pointing(az_el{-M_PI / 2, 0}), glm::vec3{-1, 0, 0}, eps));
    ASSERT_TRUE(nearby(
            compute_pointing(az_el{M_PI / 2, 0}), glm::vec3{1, 0, 0}, eps));
    ASSERT_TRUE(
            nearby(compute_pointing(az_el{-M_PI, 0}), glm::vec3{0, 0, 1}, eps));
    ASSERT_TRUE(
            nearby(compute_pointing(az_el{M_PI, 0}), glm::vec3{0, 0, 1}, eps));

    ASSERT_TRUE(nearby(
            compute_pointing(az_el{0, M_PI / 2}), glm::vec3{0, 1, 0}, eps));
    ASSERT_TRUE(nearby(
            compute_pointing(az_el{0, -M_PI / 2}), glm::vec3{0, -1, 0}, eps));
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

std::ostream& operator<<(std::ostream& os, const glm::vec3& p) {
    util::Bracketer b{os};
    return os << p.x << ", " << p.y << ", " << p.z;
}

}  // namespace

TEST(orientation, matrix) {
    ASSERT_EQ(orientation{}.get_matrix(), glm::mat4{});
}

TEST(orientation, pointing) {
    const auto test = [](auto v) {
        ASSERT_TRUE(nearby(
                transform(orientation{glm::vec3{0, 0, -1}}, v), v, 0.00001))
                << v.x << ", " << v.y << ", " << v.z;
    };

    test(glm::vec3{0, 0, -0.1});
    test(glm::vec3{0, 0, 0.1});

    test(glm::vec3{-0.1, 0, 0});
    test(glm::vec3{0.1, 0, 0});

    test(glm::vec3{0, -0.1, 0});
    test(glm::vec3{0, 0.1, 0});

    for (auto i = 0; i != 100; ++i) {
        const auto v = glm::sphericalRand(1.0f);

        ASSERT_TRUE(nearby(
                transform(orientation{v}, v), glm::vec3{0, 0, -1}, 0.00001));
        test(v);
    }
}

TEST(orientation, combine) {
    const auto eps = 0.00001;
    ASSERT_TRUE(nearby(combine(orientation{glm::vec3{1, 0, 0}},
                               orientation{glm::vec3{1, 0, 0}})
                               .get_pointing(),
                       glm::vec3{0, 0, 1},
                       eps));
    ASSERT_TRUE(nearby(combine(orientation{glm::vec3{-1, 0, 0}},
                               orientation{glm::vec3{-1, 0, 0}})
                               .get_pointing(),
                       glm::vec3{0, 0, 1},
                       eps));
    ASSERT_TRUE(nearby(combine(orientation{glm::vec3{1, 0, 0}},
                               orientation{glm::vec3{-1, 0, 0}})
                               .get_pointing(),
                       glm::vec3{0, 0, -1},
                       eps));
    ASSERT_TRUE(nearby(combine(orientation{glm::vec3{-1, 0, 0}},
                               orientation{glm::vec3{1, 0, 0}})
                               .get_pointing(),
                       glm::vec3{0, 0, -1},
                       eps));
    ASSERT_TRUE(nearby(combine(orientation{glm::vec3{0, 0, -1}},
                               orientation{glm::vec3{0, 0, -1}})
                               .get_pointing(),
                       glm::vec3{0, 0, -1},
                       eps));
    ASSERT_TRUE(nearby(combine(orientation{glm::vec3{0, 0, 1}},
                               orientation{glm::vec3{0, 0, 1}})
                               .get_pointing(),
                       glm::vec3{0, 0, -1},
                       eps));
}
