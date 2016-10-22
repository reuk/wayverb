#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"

#include "gtest/gtest.h"

inline auto check_nearby_vectors(const glm::vec3& a, const glm::vec3& b) {
    ASSERT_NEAR(glm::distance(a, b), 0, 0.00001);
}

TEST(attenuator, transform) {
    {
        const auto test = [](auto vec) {
            ASSERT_EQ(attenuator::transform(
                              glm::vec3{0, 0, 1}, glm::vec3{0, 1, 0}, vec),
                      vec);
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
            check_nearby_vectors(
                    attenuator::transform(
                            glm::vec3{1, 0, 0}, glm::vec3{0, 1, 0}, a),
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
                    attenuator::transform(
                            glm::vec3{0, 0, 1}, glm::vec3{0, -1, 0}, a),
                    b);
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
                    attenuator::transform(
                            glm::vec3{1, 0, 0}, glm::vec3{0, -1, 0}, a),
                    b);
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
    ASSERT_NE(attenuation(attenuator::hrtf{glm::vec3{0, 0, 1},
                                           glm::vec3{0, 1, 0},
                                           attenuator::hrtf::channel::left},
                          glm::vec3{0, 0, 1}),
              bands_type{});
    ASSERT_NE(attenuation(attenuator::hrtf{glm::vec3{0, 0, 1},
                                           glm::vec3{0, 1, 0},
                                           attenuator::hrtf::channel::right},
                          glm::vec3{0, 0, 1}),
              bands_type{});
}

TEST(attenuator, microphone) {
    {
        const attenuator::microphone mic{glm::vec3{1, 0, 0}, 0};
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
        const attenuator::microphone mic{glm::vec3{1, 0, 0}, 0.5};
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
        const attenuator::microphone mic{glm::vec3{1, 0, 0}, 1};
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
                attenuator::hrtf{pointing, up, channel, radius},
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
