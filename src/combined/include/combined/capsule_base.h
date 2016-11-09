#pragma once

#include "utilities/aligned/vector.h"

#include "glm/glm.hpp"

#include <memory>

namespace wayverb {

namespace core {
class orientable;
namespace attenuator {
class hrtf;
class microphone;
}  // namespace attenuator
}  // namespace core

namespace combined {

class intermediate;

class capsule_base {
public:
    capsule_base() = default;
    capsule_base(const capsule_base&) = default;
    capsule_base(capsule_base&&) noexcept = default;
    capsule_base& operator=(const capsule_base&) = default;
    capsule_base& operator=(capsule_base&&) noexcept = default;
    virtual ~capsule_base() noexcept = default;

    virtual std::unique_ptr<capsule_base> clone() const = 0;
    virtual util::aligned::vector<float> postprocess(
            const intermediate& intermediate, double sample_rate) const = 0;
};

std::unique_ptr<capsule_base> make_capsule_ptr(
        const core::attenuator::hrtf& attenuator,
        const core::orientable& orientation);

std::unique_ptr<capsule_base> make_capsule_ptr(
        const core::attenuator::microphone& attenuator,
        const core::orientable& orientation);

}  // namespace combined
}  // namespace wayverb
