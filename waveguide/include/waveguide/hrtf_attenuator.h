#pragma once

#include "common/aligned/vector.h"
#include "common/hrtf_utils.h"
#include "waveguide/waveguide.h"

#include "glm/glm.hpp"

namespace waveguide {

class HrtfAttenuator final {
public:
    aligned::vector<aligned::vector<float>> process(
            const aligned::vector<RunStepResult>& input,
            const glm::vec3& direction,
            const glm::vec3& up,
            HrtfChannel channel) const;
};

}  // namespace waveguide
