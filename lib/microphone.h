#pragma once

#include "vec.h"
#include "waveguide.h"

#include <vector>

class Microphone {
public:
    Microphone(const Vec3f& direction, float shape = 0);
    virtual ~Microphone() noexcept = default;

    float attenuation(const Vec3f& incident) const;

    std::vector<float> process(const std::vector<RunStepResult>& input) const;

    Vec3f get_direction() const;
    float get_shape() const;

private:
    Vec3f direction;
    float shape;
};
