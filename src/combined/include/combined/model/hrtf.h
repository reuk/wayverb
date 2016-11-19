#pragma once

#include "combined/model/member.h"

#include "core/attenuator/hrtf.h"
#include "core/serialize/attenuators.h"

namespace wayverb {
namespace combined {
namespace model {

class hrtf final : public basic_member<hrtf> {
public:
    hrtf() = default;
    hrtf(const core::orientation& o, core::attenuator::hrtf::channel channel);

    void set_orientation(const core::orientation& o);
    void set_channel(core::attenuator::hrtf::channel channel);

    core::attenuator::hrtf get() const;

    template <typename Archive>
    void serialize(Archive& archive) {
        archive(hrtf_);
    }

private:
    core::attenuator::hrtf hrtf_;
};

}  // namespace model
}  // namespace combined
}  // namespace wayverb
