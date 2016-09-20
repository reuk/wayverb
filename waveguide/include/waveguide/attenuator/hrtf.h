#pragma once

#include "waveguide/postprocessor/microphone.h"

#include "common/aligned/vector.h"
#include "common/hrtf_utils.h"

#include "glm/glm.hpp"

namespace waveguide {
namespace attenuator {

class hrtf final {
public:
    hrtf(const glm::vec3& direction,
         const glm::vec3& up,
         hrtf_channel channel);

    aligned::vector<volume_type>
    process(const aligned::vector<
            waveguide::postprocessor::microphone_state::output>& input) const;

private:
    glm::vec3 direction_;
    glm::vec3 up_;
    hrtf_channel channel_;
};

}  // namespace attenuator
}  // namespace waveguide
