#pragma once

#include "attenuator.h"

class Microphone : public Attenuator {
public:
    explicit Microphone(const glm::vec3& pointing = glm::vec3(1, 0, 0),
                        float shape = 0);

    void set_pointing(const glm::vec3& p);
    glm::vec3 get_pointing() const;

    void set_shape(float f);
    float get_shape() const;

    float attenuation(const glm::vec3& incident) const;

    std::vector<float> process(
            const std::vector<RunStepResult>& input) const override;

    static const Microphone omni;

private:
    glm::vec3 pointing;
    float shape;
};
