#include "waveguide/attenuator/microphone.h"

#include "common/stl_wrappers.h"

namespace {

float attenuation(const glm::vec3& incident,
                  const glm::vec3& pointing,
                  float shape) {
    return (1 - shape) + shape * glm::dot(pointing, glm::normalize(incident));
}

}  // namespace

namespace waveguide {
namespace attenuator {

aligned::vector<float> microphone::process(
        const aligned::vector<run_step_output>& input,
        const glm::vec3& pointing,
        float shape) const {
    aligned::vector<float> ret;
    ret.reserve(input.size());
    proc::transform(input, std::back_inserter(ret), [pointing, shape](auto i) {
        auto mag = glm::length(i.intensity);
        if (mag == 0) {
            return 0.0f;
        }
        mag = std::sqrt(
                mag *
                std::pow(attenuation(i.intensity, pointing, shape), 2.0f));
        return std::copysign(mag, i.pressure);
    });

    //  TODO filter with diffuse-field-response filter here
    //  make sure to use zero-phase filtering
    return ret;
}

}  // namespace attenuator
}  // namespace waveguide
