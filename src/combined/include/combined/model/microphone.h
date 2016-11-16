#pragma once

#include "combined/model/member.h"

#include "core/attenuator/microphone.h"
#include "core/serialize/attenuators.h"

namespace wayverb {
namespace combined {
namespace model {

class microphone final : public basic_member<microphone> {
public:
    microphone() = default;
    microphone(const core::orientation& o, float shape);

    void set_orientation(const core::orientation& o);
    void set_shape(float shape);

    core::attenuator::microphone get() const;

    template <typename Archive>
    void load(Archive& archive) {
        archive(microphone_);
    }

    template <typename Archive>
    void save(Archive& archive) const {
        archive(microphone_);
    }

private:
    core::attenuator::microphone microphone_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
