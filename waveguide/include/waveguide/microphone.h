#pragma once

#include "attenuator.h"

#include "common/attenuation_configs.h"

class Microphone : public Attenuator, public config::Microphone {
public:
    constexpr explicit Microphone(const Vec3f& facing, float shape = 0)
            : config::Microphone{facing, shape} {
    }

    constexpr float attenuation(const Vec3f& incident) const {
        return (1 - shape) + shape * facing.dot(incident.normalized());
    }
    std::vector<float> process(
        const std::vector<RunStepResult>& input) const override;

    static const Microphone omni;
};
