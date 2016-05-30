#pragma once

#include "attenuator.h"

#include "common/attenuation_configs.h"

class Microphone : public Attenuator, public config::Microphone {
public:
    explicit Microphone(const glm::vec3& facing, float shape = 0)
            : config::Microphone{facing, shape} {
    }

    float attenuation(const glm::vec3& incident) const {
        return (1 - shape) + shape * glm::dot(facing, glm::normalize(incident));
    }
    std::vector<float> process(
        const std::vector<RunStepResult>& input) const override;

    static const Microphone omni;
};
