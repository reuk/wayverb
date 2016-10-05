#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"

#include "gtest/gtest.h"

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
