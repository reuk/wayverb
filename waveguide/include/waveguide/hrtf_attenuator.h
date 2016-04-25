#pragma once

#include "attenuator.h"

class HrtfAttenuator : public Attenuator {
public:
    HrtfAttenuator(const Vec3f& direction,
                   const Vec3f& up,
                   int channel,
                   float sr);

    float attenuation(const Vec3f& incident, int band) const;
    std::vector<float> process(
        const std::vector<RunStepResult>& input) const override;

    Vec3f get_direction() const;
    Vec3f get_up() const;

private:
    Vec3f direction;
    Vec3f up;
    int channel;
    float sr;
};
