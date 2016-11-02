#include "raytracer/attenuator.h"

#include "core/attenuator/hrtf.h"
#include "core/attenuator/microphone.h"

#include "gtest/gtest.h"

TEST(attenuator, microphone) {
    {
        const auto mic = core::attenuator::microphone{glm::vec3{1, 0, 0}, 0};
        const auto receiver = glm::vec3{0, 0, 0};
        const auto calculate = [&](const auto& pos) {
            return raytracer::attenuate(
                           mic,
                           receiver,
                           impulse<1>{
                                   cl_float1{{1}}, core::to_cl_float3(pos), 1})
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
        const auto mic = core::attenuator::microphone{glm::vec3{1, 0, 0}, 0.5};
        const auto receiver = glm::vec3{0, 0, 0};
        const auto calculate = [&](const auto& pos) {
            return raytracer::attenuate(
                           mic,
                           receiver,
                           impulse<1>{
                                   cl_float1{{1}}, core::to_cl_float3(pos), 1})
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
        const auto mic = core::attenuator::microphone{glm::vec3{1, 0, 0}, 1};
        const auto receiver = glm::vec3{0, 0, 0};
        const auto calculate = [&](const auto& pos) {
            return raytracer::attenuate(
                           mic,
                           receiver,
                           impulse<1>{
                                   cl_float1{{1}}, core::to_cl_float3(pos), 1})
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

TEST(attenuator, hrtf) {
    {
        constexpr auto receiver_position = glm::vec3{0, 0, 0};
        constexpr auto pointing = glm::vec3{0, 0, 1};
        constexpr auto up = glm::vec3{0, 1, 0};

        constexpr auto radius = 0.1f;

        const auto left = core::attenuator::hrtf{
                pointing, up, core::attenuator::hrtf::channel::left, radius};
        const auto right = core::attenuator::hrtf{
                pointing, up, core::attenuator::hrtf::channel::right, radius};

        constexpr auto tolerance = 0.0000001;

        {
            constexpr auto impulse_position = glm::vec3{-1, 0, 0};
            const auto impulse = make_impulse(
                    core::bands_type{{1, 1, 1, 1, 1, 1, 1, 1}},
                    core::to_cl_float3(impulse_position),
                    glm::distance(impulse_position, receiver_position));

            ASSERT_NEAR(
                    glm::distance(impulse_position, receiver_position) - radius,
                    raytracer::attenuate(left, receiver_position, impulse)
                            .distance,
                    tolerance);

            ASSERT_NEAR(
                    glm::distance(impulse_position, receiver_position) + radius,
                    raytracer::attenuate(right, receiver_position, impulse)
                            .distance,
                    tolerance);
        }

        {
            constexpr auto impulse_position = glm::vec3{1, 0, 0};
            const auto impulse = make_impulse(
                    core::bands_type{{1, 1, 1, 1, 1, 1, 1, 1}},
                    core::to_cl_float3(impulse_position),
                    glm::distance(impulse_position, receiver_position));

            ASSERT_NEAR(
                    glm::distance(impulse_position, receiver_position) + radius,
                    raytracer::attenuate(left, receiver_position, impulse)
                            .distance,
                    tolerance);

            ASSERT_NEAR(
                    glm::distance(impulse_position, receiver_position) - radius,
                    raytracer::attenuate(right, receiver_position, impulse)
                            .distance,
                    tolerance);
        }

        {
            constexpr auto impulse_position = glm::vec3{0, 2, 0};
            const auto impulse = make_impulse(
                    core::bands_type{{1, 1, 1, 1, 1, 1, 1, 1}},
                    core::to_cl_float3(impulse_position),
                    glm::distance(impulse_position, receiver_position));

            ASSERT_EQ(raytracer::attenuate(left, receiver_position, impulse)
                              .distance,
                      raytracer::attenuate(right, receiver_position, impulse)
                              .distance);
        }
    }
}
