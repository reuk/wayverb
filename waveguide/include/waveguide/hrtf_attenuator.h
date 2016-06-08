#pragma once

#include "attenuator.h"

#include "glm/glm.hpp"

class HrtfAttenuator : public Attenuator {
public:
    HrtfAttenuator(const glm::vec3& direction,
                   const glm::vec3& up,
                   int channel,
                   float sr);

    float attenuation(const glm::vec3& incident, int band) const;
    std::vector<float> process(
            const std::vector<RunStepResult>& input) const override;

    glm::vec3 get_direction() const;
    glm::vec3 get_up() const;

private:
    glm::vec3 direction;
    glm::vec3 up;
    int channel;
    float sr;
};
