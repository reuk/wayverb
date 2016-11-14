#include "combined/model/hrtf.h"

#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

void hrtf::set_orientation(const core::orientable& o) {
    hrtf_.orientable = o;
    notify();
}

void hrtf::set_channel(core::attenuator::hrtf::channel channel) {
    hrtf_.set_channel(channel);
    notify();
}

core::attenuator::hrtf hrtf::get() const { return hrtf_; }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
