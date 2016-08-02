#pragma once

#include "common/aligned/vector.h"
#include "waveguide/waveguide.h"

#include "glm/fwd.hpp"

namespace waveguide {
namespace attenuator {

class microphone final {
public:
    aligned::vector<float> process(
            const aligned::vector<run_step_output>& input,
            const glm::vec3& pointing,
            float shape) const;
};

}  // namespace attenuator
}  // namespace waveguide
