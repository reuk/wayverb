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

    void set_orientation(const core::orientable& o);
    void set_shape(double shape);

    core::attenuator::microphone get() const;

    template <typename Archive>
    void load(Archive& archive) {
        archive(microphone_);
        notify();
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
