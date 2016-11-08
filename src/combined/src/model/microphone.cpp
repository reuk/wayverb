#include "combined/model/microphone.h"

#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

void microphone::set_orientation(float azimuth, float elevation) {
    microphone_.set_pointing(compute_pointing(core::az_el{azimuth, elevation}));
    notify();
}

void microphone::set_shape(double shape) {
    microphone_.set_shape(shape);
    notify();
}

core::attenuator::microphone microphone::get() const { return microphone_; }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
