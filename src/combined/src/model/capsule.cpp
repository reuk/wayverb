#include "combined/model/capsule.h"

#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

void capsule::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

std::string capsule::get_name() const { return name_; }

void capsule::set_mode(mode mode) {
    mode_ = mode;
    notify();
}

capsule::mode capsule::get_mode() const { return mode_; }

microphone& capsule::microphone() { return get<microphone_t>(); }
const microphone& capsule::microphone() const { return get<microphone_t>(); }

hrtf& capsule::hrtf() { return get<hrtf_t>(); }
const hrtf& capsule::hrtf() const { return get<hrtf_t>(); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
