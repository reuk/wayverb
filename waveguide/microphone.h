#pragma once

#include "attenuator.h"

class Microphone : public Attenuator {
public:
    Microphone(const Vec3f& direction, float shape = 0);
    virtual ~Microphone() noexcept = default;

    float attenuation(const Vec3f& incident) const;
    std::vector<float> process(
        const std::vector<RunStepResult>& input) const override;

    Vec3f get_direction() const;
    float get_shape() const;

    static const Microphone omni;

private:
    Vec3f direction;
    float shape;
};
