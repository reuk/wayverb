#include "combined/model/hrtf.h"

#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

hrtf::hrtf(const core::orientation& o, core::attenuator::hrtf::channel channel)
        : hrtf_{o, channel} {}

void hrtf::set_orientation(const core::orientation& o) {
    hrtf_.orientation = o;
    notify();
}

void hrtf::set_channel(core::attenuator::hrtf::channel channel) {
    hrtf_.set_channel(channel);
    notify();
}

core::attenuator::hrtf hrtf::get() const { return hrtf_; }

bool operator==(const hrtf& a, const hrtf& b) {
    return a.get() == b.get();
}

bool operator!=(const hrtf& a, const hrtf& b) {
    return !(a == b);
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
