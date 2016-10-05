#include "waveguide/attenuator.h"

#include "common/attenuator/microphone.h"

#include "gtest/gtest.h"

/*
TEST(attenuate, attenuate) {
    const microphone omni{glm::vec3{0, 0, 1}, 0};
    const auto acoustic_impedance{400.0f};

    const auto run_attenuate{[=](auto i) {
        return waveguide::attenuate(omni, acoustic_impedance, i);
    }};

    using output = waveguide::postprocessor::directional_receiver::output;

    ASSERT_EQ(run_attenuate(output{glm::vec3{0, 0, 0}, 1.0f}), 1.0f);
}
*/
