#pragma once

#include "core/orientable.h"

namespace wayverb {
namespace core {
namespace attenuator {

/// Super-simple class which maintains microphone invariants.
class microphone final {
public:
    using orientable_t = class orientable;
    explicit microphone(const orientable_t& o = orientable_t(),
                        float shape = 0.0f);

    float get_shape() const;
    void set_shape(float shape);

    template <typename Archive>
    void serialize(Archive&);

    orientable_t orientable;
    
private:
    float shape_{0.0f};
};

float attenuation(const microphone& mic, const glm::vec3& incident);

}  // namespace attenuator
}  // namespace core
}  // namespace wayverb
