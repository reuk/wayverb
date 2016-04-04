#include "microphone.h"
#include "stl_wrappers.h"

#include <iostream>

Microphone::Microphone(const Vec3f& direction, float shape)
        : direction(direction.normalized())
        , shape(shape) {
}

float Microphone::attenuation(const Vec3f& incident) const {
    return (1 - shape) + shape * direction.dot(incident.normalized());
}

Vec3f Microphone::get_direction() const {
    return direction;
}
float Microphone::get_shape() const {
    return shape;
}

std::vector<float> Microphone::process(
    const std::vector<RunStepResult>& input) const {
    std::vector<float> ret(input.size());
    proc::transform(input,
                    ret.begin(),
                    [this](auto i) {
                        //  TODO DEFINITELY CHECK THIS
                        //  RUN TESTS YEAH
                        auto mag = i.intensity.mag();
                        if (mag == 0)
                            return 0.0f;
                        mag = sqrt(mag * pow(attenuation(i.intensity), 2));
                        return std::copysign(mag, i.pressure);
                    });

    //  TODO filter with diffuse-field-response filter here
    //  make sure to use zero-phase filtering
    return ret;
}

const Microphone Microphone::omni{Vec3f(1, 0, 0)};
