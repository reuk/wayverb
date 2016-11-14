#include "combined/model/microphone.h"

#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

void microphone::set_orientation(const core::orientable& o) {
    microphone_.orientable = o;
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
