#include "waveguide/microphone.h"

#include "common/stl_wrappers.h"

#include <iostream>

Microphone::Microphone(const glm::vec3& pointing, float shape)
        : pointing(glm::normalize(pointing))
        , shape(shape) {
}

void Microphone::set_pointing(const glm::vec3& p) {
    pointing = p;
}
glm::vec3 Microphone::get_pointing() const {
    return pointing;
}

void Microphone::set_shape(float f) {
    shape = f;
}
float Microphone::get_shape() const {
    return shape;
}

float Microphone::attenuation(const glm::vec3& incident) const {
    return (1 - shape) + shape * glm::dot(pointing, glm::normalize(incident));
}

std::vector<float> Microphone::process(
        const std::vector<RunStepResult>& input) const {
    std::vector<float> ret;
    ret.reserve(input.size());
    proc::transform(input, std::back_inserter(ret), [this](auto i) {
        //  TODO DEFINITELY CHECK THIS
        //  RUN TESTS YEAH
        auto mag = glm::length(i.intensity);
        if (mag == 0) {
            return 0.0f;
        }
        mag = std::sqrt(mag * std::pow(attenuation(i.intensity), 2.0f));
        return std::copysign(mag, i.pressure);
    });

    //  TODO filter with diffuse-field-response filter here
    //  make sure to use zero-phase filtering
    return ret;
}

const Microphone Microphone::omni{glm::vec3(1, 0, 0)};
