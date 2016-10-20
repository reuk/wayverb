#include "raytracer/attenuator.h"

#include "common/attenuator/microphone.h"

#include "gtest/gtest.h"

TEST(attenuator, microphone) {
    {
        const auto mic = attenuator::microphone{glm::vec3{1, 0, 0}, 0};
        const auto receiver = glm::vec3{0, 0, 0};
        const auto calculate = [&](const auto& pos) {
            return raytracer::attenuate(
                           mic,
                           receiver,
                           impulse<1>{cl_float1{{1}}, to_cl_float3(pos), 1})
                    .volume.s[0];
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
        const auto mic = attenuator::microphone{glm::vec3{1, 0, 0}, 0.5};
        const auto receiver = glm::vec3{0, 0, 0};
        const auto calculate = [&](const auto& pos) {
            return raytracer::attenuate(
                           mic,
                           receiver,
                           impulse<1>{cl_float1{{1}}, to_cl_float3(pos), 1})
                    .volume.s[0];
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
        const auto mic = attenuator::microphone{glm::vec3{1, 0, 0}, 1};
        const auto receiver = glm::vec3{0, 0, 0};
        const auto calculate = [&](const auto& pos) {
            return raytracer::attenuate(
                           mic,
                           receiver,
                           impulse<1>{cl_float1{{1}}, to_cl_float3(pos), 1})
                    .volume.s[0];
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

TEST(attenuator, hrtf) { ASSERT_TRUE(false); }
