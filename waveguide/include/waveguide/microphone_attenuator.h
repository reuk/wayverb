#pragma once

#include "common/aligned/vector.h"
#include "waveguide/waveguide.h"

#include "glm/fwd.hpp"

namespace waveguide {

class MicrophoneAttenuator final {
public:
    aligned::vector<float> process(const aligned::vector<RunStepResult>& input,
                                   const glm::vec3& pointing,
                                   float shape) const;
};

}  // namespace waveguide