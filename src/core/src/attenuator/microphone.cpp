#include "core/attenuator/microphone.h"
#include "utilities/range.h"

namespace wayverb {
namespace core {
namespace attenuator {

microphone::microphone(const glm::vec3& pointing, float shape)
        : orientable_{pointing}
        , shape_{glm::clamp(shape, 0.0f, 1.0f)} {}

glm::vec3 microphone::get_pointing() const {
    return orientable_.get_pointing();
}
float microphone::get_shape() const { return shape_; }

void microphone::set_pointing(const glm::vec3& pointing) {
    orientable_.set_pointing(pointing);
}

void microphone::set_shape(float shape) {
    shape_ = clamp(shape, util::make_range(0.0f, 1.0f));
}

float attenuation(const microphone& mic, const glm::vec3& incident) {
    if (const auto l = glm::length(incident)) {
        return (1 - mic.get_shape()) +
               mic.get_shape() * glm::dot(mic.get_pointing(), incident / l);
    }
    return 0;
}

}  // namespace attenuator
}  // namespace core
}  // namespace wayverb
