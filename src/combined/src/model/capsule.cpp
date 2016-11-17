#include "combined/model/capsule.h"

#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

capsule::capsule(std::string name,
                 mode mode,
                 microphone_t microphone,
                 hrtf_t hrtf)
        : base_type{std::move(microphone), std::move(hrtf)}
        , name_{std::move(name)}
        , mode_{mode} {}

capsule::capsule(std::string name, microphone_t microphone)
        : base_type{std::move(microphone), hrtf_t{}}
        , name_{std::move(name)}
        , mode_{mode::microphone} {}

capsule::capsule(std::string name, hrtf_t hrtf)
        : base_type{microphone_t{}, std::move(hrtf)}
        , name_{std::move(name)}
        , mode_{mode::hrtf} {}

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

shared_value<capsule::microphone_t>& capsule::microphone() { return get<0>(); }
const shared_value<capsule::microphone_t>& capsule::microphone() const {
    return get<0>();
}

shared_value<capsule::hrtf_t>& capsule::hrtf() { return get<1>(); }
const shared_value<capsule::hrtf_t>& capsule::hrtf() const { return get<1>(); }

core::orientation get_orientation(const capsule& capsule) {
    switch (capsule.get_mode()) {
        case capsule::mode::microphone:
            return capsule.microphone()->get().orientation;
        case capsule::mode::hrtf: return capsule.hrtf()->get().orientation;
    }
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
