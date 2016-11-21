#include "combined/model/microphone.h"

#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

microphone::microphone(const core::orientation& o, float shape)
        : microphone_{o, shape} {}

void microphone::swap(microphone& other) noexcept {
    using std::swap;
    swap(microphone_, other.microphone_);
}

microphone& microphone::operator=(microphone other) {
    base_type::operator=(other);
    swap(other);
    notify();
    return *this;
}

void microphone::set_orientation(const core::orientation& o) {
    microphone_.orientation = o;
    notify();
}

void microphone::set_shape(float shape) {
    microphone_.set_shape(shape);
    notify();
}

core::attenuator::microphone microphone::get() const { return microphone_; }

bool operator==(const microphone& a, const microphone& b) {
    return a.get() == b.get();
}

bool operator!=(const microphone& a, const microphone& b) {
    return !(a == b);
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
