#pragma once

#include "glm/glm.hpp"

/// Super-simple class which maintains microphone invariants.
class microphone final {
public:
    microphone(const glm::vec3& pointing, float shape);

    glm::vec3 get_pointing() const;
    float get_shape() const;

    void set_pointing(const glm::vec3& pointing);
    void set_shape(float shape);

private:
    glm::vec3 pointing_;
    float shape_;
};

float attenuation(const microphone& mic, const glm::vec3& incident);
