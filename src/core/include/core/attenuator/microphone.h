#pragma once

#include "core/orientable.h"
 
namespace wayverb {
namespace core {
namespace attenuator {

/// Super-simple class which maintains microphone invariants.
class microphone final {
public:
    microphone() = default;
    microphone(const glm::vec3& pointing, float shape);

    glm::vec3 get_pointing() const;
    float get_shape() const;

    void set_pointing(const glm::vec3& pointing);
    void set_shape(float shape);

private:
    orientable orientable_;
    float shape_{0.0f};
};

float attenuation(const microphone& mic, const glm::vec3& incident);

}  // namespace attenuator
}  // namespace core
}  // namespace wayverb
