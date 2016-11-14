#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"

#include "gtest/gtest.h"

#include <ostream>

using namespace wayverb::core;

namespace {
std::ostream& operator<<(std::ostream& os, const glm::vec3& p) {
    return os << p.x << ", " << p.y << ", " << p.z;
}

inline auto check_nearby_vectors(const glm::vec3& a, const glm::vec3& b) {
    std::cout << "a: " << a << ", b: " << b << '\n';
    ASSERT_NEAR(glm::distance(a, b), 0, 0.00001);
}
}

TEST(attenuator, transform) {
    {
        const auto test = [](auto vec) {
            const auto a = transform(orientation{}, vec);
            const auto b = vec;
            check_nearby_vectors(a, b);
        };

        test(glm::vec3{1, 0, 0});
        test(glm::vec3{-1, 0, 0});
        test(glm::vec3{0, 1, 0});
        test(glm::vec3{0, -1, 0});
        test(glm::vec3{0, 0, 1});
        test(glm::vec3{0, 0, -1});
    }

    {
        const auto test = [](auto a, auto b) {
            check_nearby_vectors(transform(orientation{{1, 0, 0}, {0, 1, 0}}, a),
                                 b);
        };

        test(glm::vec3{1, 0, 0}, glm::vec3{0, 0, 1});
        test(glm::vec3{-1, 0, 0}, glm::vec3{0, 0, -1});
        test(glm::vec3{0, 1, 0}, glm::vec3{0, 1, 0});
        test(glm::vec3{0, -1, 0}, glm::vec3{0, -1, 0});
        test(glm::vec3{0, 0, 1}, glm::vec3{-1, 0, 0});
        test(glm::vec3{0, 0, -1}, glm::vec3{1, 0, 0});
    }

    {
        const auto test = [](auto a, auto b) {
            check_nearby_vectors(
                    transform(orientation{{0, 0, 1}, {0, -1, 0}}, a), b);
        };

        test(glm::vec3{1, 0, 0}, glm::vec3{-1, 0, 0});
        test(glm::vec3{-1, 0, 0}, glm::vec3{1, 0, 0});
        test(glm::vec3{0, 1, 0}, glm::vec3{0, -1, 0});
        test(glm::vec3{0, -1, 0}, glm::vec3{0, 1, 0});
        test(glm::vec3{0, 0, 1}, glm::vec3{0, 0, 1});
        test(glm::vec3{0, 0, -1}, glm::vec3{0, 0, -1});
    }

    {
        const auto test = [](auto a, auto b) {
            check_nearby_vectors(
                    transform(orientation{{1, 0, 0}, {0, -1, 0}}, a), b);
        };

        test(glm::vec3{1, 0, 0}, glm::vec3{0, 0, 1});
        test(glm::vec3{-1, 0, 0}, glm::vec3{0, 0, -1});
        test(glm::vec3{0, 1, 0}, glm::vec3{0, -1, 0});
        test(glm::vec3{0, -1, 0}, glm::vec3{0, 1, 0});
        test(glm::vec3{0, 0, 1}, glm::vec3{1, 0, 0});
        test(glm::vec3{0, 0, -1}, glm::vec3{-1, 0, 0});
    }
}

TEST(attenuator, hrtf) {
    ASSERT_NE(attenuation(attenuator::hrtf{orientation{{0, 0, 1}, {0, 1, 0}},
                                           attenuator::hrtf::channel::left},
                          glm::vec3{0, 0, 1}),
              bands_type{});

    ASSERT_NE(attenuation(attenuator::hrtf{orientation{{0, 0, 1}, {0, 1, 0}},
                                           attenuator::hrtf::channel::right},
                          glm::vec3{0, 0, 1}),
              bands_type{});
}

TEST(attenuator, microphone) {
    {
        const attenuator::microphone mic{orientation{{1, 0, 0}}, 0};
        const auto calculate = [&](const auto& dir) {
            return attenuation(mic, dir);
        };
        ASSERT_EQ(calculate(glm::vec3{0, 0, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{1, 0, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, 1, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, 0, 1}), 1);
        ASSERT_EQ(calculate(glm::vec3{-1, 0, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, -1, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, 0, -1}), 1);
    }

    {
        const attenuator::microphone mic{orientation{{1, 0, 0}}, 0.5};
        const auto calculate = [&](const auto& dir) {
            return attenuation(mic, dir);
        };
        ASSERT_EQ(calculate(glm::vec3{0, 0, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{1, 0, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, 1, 0}), 0.5);
        ASSERT_EQ(calculate(glm::vec3{0, 0, 1}), 0.5);
        ASSERT_EQ(calculate(glm::vec3{-1, 0, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{0, -1, 0}), 0.5);
        ASSERT_EQ(calculate(glm::vec3{0, 0, -1}), 0.5);
    }

    {
        const attenuator::microphone mic{orientation{{1, 0, 0}}, 1};
        const auto calculate = [&](const auto& dir) {
            return attenuation(mic, dir);
        };
        ASSERT_EQ(calculate(glm::vec3{0, 0, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{1, 0, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, 1, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{0, 0, 1}), 0);
        ASSERT_EQ(calculate(glm::vec3{-1, 0, 0}), -1);
        ASSERT_EQ(calculate(glm::vec3{0, -1, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{0, 0, -1}), 0);
    }
}

TEST(attenuator, hrtf_ear_position) {
    const auto radius = 0.1f;

    const auto test = [&](auto pointing, auto up, auto channel, auto pos) {
        const auto ear_pos = get_ear_position(
                attenuator::hrtf{orientation{pointing, up}, channel, radius},
                glm::vec3{0, 0, 0});
        check_nearby_vectors(ear_pos, pos);
    };

    test(glm::vec3{0, 0, 1},
         glm::vec3{0, 1, 0},
         attenuator::hrtf::channel::left,
         glm::vec3{-radius, 0, 0});

    test(glm::vec3{0, 0, 1},
         glm::vec3{0, 1, 0},
         attenuator::hrtf::channel::right,
         glm::vec3{radius, 0, 0});

    test(glm::vec3{1, 0, 0},
         glm::vec3{0, 1, 0},
         attenuator::hrtf::channel::left,
         glm::vec3{0, 0, -radius});

    test(glm::vec3{1, 0, 0},
         glm::vec3{0, 1, 0},
         attenuator::hrtf::channel::right,
         glm::vec3{0, 0, radius});

    test(glm::vec3{1, 0, 0},
         glm::vec3{0, -1, 0},
         attenuator::hrtf::channel::left,
         glm::vec3{0, 0, -radius});

    test(glm::vec3{1, 0, 0},
         glm::vec3{0, -1, 0},
         attenuator::hrtf::channel::right,
         glm::vec3{0, 0, radius});
}
