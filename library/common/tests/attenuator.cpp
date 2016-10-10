#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"

#include "gtest/gtest.h"

TEST(attenuator, transform) {
    {
        const auto test{[](auto vec) {
            ASSERT_EQ(transform(glm::vec3{0, 0, 1}, glm::vec3{0, 1, 0}, vec),
                      vec);
        }};
        test(glm::vec3{1, 0, 0});
        test(glm::vec3{-1, 0, 0});
        test(glm::vec3{0, 1, 0});
        test(glm::vec3{0, -1, 0});
        test(glm::vec3{0, 0, 1});
        test(glm::vec3{0, 0, -1});
    }

    {
        const auto test{[](auto a, auto b) {
            ASSERT_NEAR(glm::distance(transform(glm::vec3{1, 0, 0},
                                                glm::vec3{0, 1, 0},
                                                a),
                                      b),
                        0,
                        0.0001);
        }};
        test(glm::vec3{1, 0, 0}, glm::vec3{0, 0, 1});
        test(glm::vec3{-1, 0, 0}, glm::vec3{0, 0, -1});
        test(glm::vec3{0, 1, 0}, glm::vec3{0, 1, 0});
        test(glm::vec3{0, -1, 0}, glm::vec3{0, -1, 0});
        test(glm::vec3{0, 0, 1}, glm::vec3{-1, 0, 0});
        test(glm::vec3{0, 0, -1}, glm::vec3{1, 0, 0});
    }
}

TEST(attenuator, compute_look_up_angles) {
    const auto test{[](auto pt, auto azel) {
        const auto result{compute_look_up_angles(pt)};
        ASSERT_NEAR(result.azimuth, azel.azimuth, 0.0001);
        ASSERT_NEAR(result.elevation, azel.elevation, 0.0001);
    }};
    test(glm::vec3{0, 0, 1}, (az_el{0, 0}));
    test(glm::vec3{0, 0, -1}, (az_el{180, 0}));

    test(glm::vec3{0, 1, 0}, (az_el{0, 90}));
    test(glm::vec3{0, -1, 0}, (az_el{0, 270}));

    test(glm::vec3{1, 0, 0}, (az_el{270, 0}));
    test(glm::vec3{-1, 0, 0}, (az_el{90, 0}));
}

TEST(attenuator, hrtf) {
    ASSERT_NE(attenuation(hrtf{glm::vec3{0, 0, 1},
                               glm::vec3{0, 1, 0},
                               hrtf::channel::left},
                          glm::vec3{0, 0, 1}),
              volume_type{});
    ASSERT_NE(attenuation(hrtf{glm::vec3{0, 0, 1},
                               glm::vec3{0, 1, 0},
                               hrtf::channel::right},
                          glm::vec3{0, 0, 1}),
              volume_type{});
}

TEST(attenuator, microphone) {
    {
        const microphone mic{glm::vec3{1, 0, 0}, 0};
        const auto calculate{
                [&](const auto& dir) { return attenuation(mic, dir); }};
        ASSERT_EQ(calculate(glm::vec3{0, 0, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{1, 0, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, 1, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, 0, 1}), 1);
        ASSERT_EQ(calculate(glm::vec3{-1, 0, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, -1, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, 0, -1}), 1);
    }

    {
        const microphone mic{glm::vec3{1, 0, 0}, 0.5};
        const auto calculate{
                [&](const auto& dir) { return attenuation(mic, dir); }};
        ASSERT_EQ(calculate(glm::vec3{0, 0, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{1, 0, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, 1, 0}), 0.5);
        ASSERT_EQ(calculate(glm::vec3{0, 0, 1}), 0.5);
        ASSERT_EQ(calculate(glm::vec3{-1, 0, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{0, -1, 0}), 0.5);
        ASSERT_EQ(calculate(glm::vec3{0, 0, -1}), 0.5);
    }

    {
        const microphone mic{glm::vec3{1, 0, 0}, 1};
        const auto calculate{
                [&](const auto& dir) { return attenuation(mic, dir); }};
        ASSERT_EQ(calculate(glm::vec3{0, 0, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{1, 0, 0}), 1);
        ASSERT_EQ(calculate(glm::vec3{0, 1, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{0, 0, 1}), 0);
        ASSERT_EQ(calculate(glm::vec3{-1, 0, 0}), -1);
        ASSERT_EQ(calculate(glm::vec3{0, -1, 0}), 0);
        ASSERT_EQ(calculate(glm::vec3{0, 0, -1}), 0);
    }
}
