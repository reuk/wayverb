#pragma once

#include "common/hrtf_utils.h"

#include "glm/glm.hpp"

#include <vector>

struct RunStepResult;

namespace waveguide {

class HrtfAttenuator final {
public:
    std::vector<std::vector<float>> process(
            const std::vector<RunStepResult>& input,
            const glm::vec3& direction,
            const glm::vec3& up,
            HrtfChannel channel) const;
};

}//namespace waveguide
