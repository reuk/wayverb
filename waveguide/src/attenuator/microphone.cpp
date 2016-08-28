#include "waveguide/attenuator/microphone.h"

#include "common/map_to_vector.h"

namespace {

float attenuation(const glm::vec3& incident,
                  const glm::vec3& pointing,
                  float shape) {
    return (1 - shape) + shape * glm::dot(pointing, glm::normalize(incident));
}

}  // namespace

namespace waveguide {
namespace attenuator {

microphone::microphone(const glm::vec3& pointing, float shape)
        : pointing_(pointing)
        , shape_(shape) {}

aligned::vector<float> microphone::process(
        const aligned::vector<run_step_output>& input) const {
    //  TODO filter with diffuse-field-response filter here
    //  make sure to use zero-phase filtering
    return map_to_vector(input, [&](auto i) {
        auto mag{glm::length(i.intensity)};
        if (mag == 0) {
            return 0.0f;
        }
        mag = std::sqrt(
                mag *
                std::pow(attenuation(i.intensity, pointing_, shape_), 2.0f));
        return std::copysign(mag, i.pressure);
    });
}

}  // namespace attenuator
}  // namespace waveguide
