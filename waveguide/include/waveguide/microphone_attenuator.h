#pragma once

#include "glm/fwd.hpp"

#include <vector>

struct RunStepResult;

namespace waveguide {

class MicrophoneAttenuator final {
public:
    std::vector<float> process(const std::vector<RunStepResult>& input,
                               const glm::vec3& pointing,
                               float shape) const;
};

}//namespace waveguide
