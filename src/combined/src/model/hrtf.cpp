#include "combined/model/hrtf.h"

#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

void hrtf::set_orientation(float azimuth, float elevation) {
    hrtf_.set_pointing(compute_pointing(core::az_el{azimuth, elevation}));
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
