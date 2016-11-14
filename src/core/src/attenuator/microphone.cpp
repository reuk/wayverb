#include "core/attenuator/microphone.h"
#include "utilities/range.h"

namespace wayverb {
namespace core {
namespace attenuator {

microphone::microphone(const orientation_t& o, float shape)
        : orientation{o}
        , shape_{clamp(shape, util::make_range(0.0f, 1.0f))} {}

float microphone::get_shape() const { return shape_; }

void microphone::set_shape(float shape) {
    shape_ = clamp(shape, util::make_range(0.0f, 1.0f));
}

float attenuation(const microphone& mic, const glm::vec3& incident) {
    if (const auto l = glm::length(incident)) {
        return (1 - mic.get_shape()) +
               mic.get_shape() *
                       glm::dot(mic.orientation.get_pointing(), incident / l);
    }
    return 0;
}

}  // namespace attenuator
}  // namespace core
}  // namespace wayverb
