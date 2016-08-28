#pragma once

#include "common/aligned/vector.h"
#include "waveguide/waveguide.h"

#include "glm/fwd.hpp"

namespace waveguide {
namespace attenuator {

class microphone final {
public:
    microphone(const glm::vec3& pointing, float shape);

    aligned::vector<float> process(
            const aligned::vector<run_step_output>& input) const;

private:
    glm::vec3 pointing_;
    float shape_;
};

}  // namespace attenuator
}  // namespace waveguide
