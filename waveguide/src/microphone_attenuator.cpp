#include "waveguide/microphone_attenuator.h"

#include "common/stl_wrappers.h"

#include <iostream>

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
        const aligned::vector<rectangular_waveguide::run_step_output>& input,
        const glm::vec3& pointing,
        float shape) const {
    aligned::vector<float> ret;
    ret.reserve(input.size());
    proc::transform(input, std::back_inserter(ret), [pointing, shape](auto i) {
        //  TODO DEFINITELY CHECK THIS
        //  RUN TESTS YEAH
        auto mag = glm::length(i.get_intensity());
        if (mag == 0) {
            return 0.0f;
        }
        mag = std::sqrt(
                mag * std::pow(attenuation(i.get_intensity(), pointing, shape),
                               2.0f));
        return std::copysign(mag, i.get_pressure());
    });

    //  TODO filter with diffuse-field-response filter here
    //  make sure to use zero-phase filtering
    return ret;
}

}  // namespace attenuator
}  // namespace waveguide
