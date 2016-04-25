#include "waveguide/microphone.h"

#include "common/stl_wrappers.h"

#include <iostream>

std::vector<float> Microphone::process(
    const std::vector<RunStepResult>& input) const {
    std::vector<float> ret(input.size());
    proc::transform(input, ret.begin(), [this](auto i) {
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
