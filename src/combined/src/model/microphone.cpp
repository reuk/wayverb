#include "combined/model/microphone.h"

#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

microphone::microphone(const core::orientation& o, float shape)
        : microphone_{o, shape} {}

void microphone::set_orientation(const core::orientation& o) {
    microphone_.orientation = o;
    notify();
}

void microphone::set_shape(float shape) {
    microphone_.set_shape(shape);
    notify();
}

core::attenuator::microphone microphone::get() const { return microphone_; }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
