#pragma once

#include "core/orientation.h"

namespace wayverb {
namespace core {
namespace attenuator {

/// Super-simple class which maintains microphone invariants.
class microphone final {
public:
    using orientation_t = class orientation;
    explicit microphone(const orientation_t& o = orientation_t(),
                        float shape = 0.0f);

    float get_shape() const;
    void set_shape(float shape);

    template <typename Archive>
    void serialize(Archive&);

    orientation_t orientation;
    
private:
    float shape_{0.0f};
};

bool operator==(const microphone& a, const microphone& b);
bool operator!=(const microphone& a, const microphone& b);

float attenuation(const microphone& mic, const glm::vec3& incident);

}  // namespace attenuator
}  // namespace core
}  // namespace wayverb
