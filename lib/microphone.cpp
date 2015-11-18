#include "microphone.h"

#include <iostream>

Microphone::Microphone(const Vec3f& direction, float shape)
        : direction(direction.normalized())
        , shape(shape) {
}

float Microphone::attenuation(const Vec3f& incident) const {
    return (1 - shape) + shape * direction.dot(incident.normalized());
}

std::vector<float> Microphone::process(
    const std::vector<RunStepResult>& input) const {
    std::vector<float> ret(input.size());
    std::transform(input.begin(),
                   input.end(),
                   ret.begin(),
                   [this](auto i) {
                       //  TODO DEFINITELY CHECK THIS
                       //  RUN TESTS YEAH
                       auto mag = i.intensity.mag();
                       if (mag == 0)
                           return 0.0f;
                       mag = sqrt(mag *
                                   pow(attenuation(i.intensity), 2));
                       return std::copysign(mag, i.pressure);
                   });
    return ret;
}
