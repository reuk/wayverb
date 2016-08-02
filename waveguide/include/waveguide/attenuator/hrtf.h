#pragma once

#include "waveguide/waveguide.h"

#include "common/aligned/vector.h"
#include "common/hrtf_utils.h"

#include "glm/glm.hpp"

namespace waveguide {
namespace attenuator {

class hrtf final {
public:
    aligned::vector<aligned::vector<float>> process(
            const aligned::vector<run_step_output>& input,
            const glm::vec3& direction,
            const glm::vec3& up,
            HrtfChannel channel) const;
};

}  // namespace attenuator
}  // namespace waveguide
