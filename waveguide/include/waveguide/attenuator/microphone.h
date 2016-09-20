#pragma once

#include "waveguide/postprocessor/microphone.h"
#include "waveguide/waveguide.h"

#include "common/aligned/vector.h"

#include "glm/fwd.hpp"

namespace waveguide {
namespace attenuator {

class microphone final {
public:
    microphone(const glm::vec3& pointing, float shape);

    aligned::vector<float>
    process(const aligned::vector<
            waveguide::postprocessor::microphone_state::output>& input) const;

private:
    glm::vec3 pointing_;
    float shape_;
};

}  // namespace attenuator
}  // namespace waveguide
