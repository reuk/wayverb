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

void capsule::swap(capsule& other) noexcept {
    using std::swap;
    swap(name_, other.name_);
    swap(mode_, other.mode_);
}

capsule& capsule::operator=(capsule other) {
    base_type::operator=(other);
    swap(other);
    notify();
    return *this;
}

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

core::orientation get_orientation(const capsule& capsule) {
    switch (capsule.get_mode()) {
        case capsule::mode::microphone:
            return capsule.microphone()->item.get().orientation;
        case capsule::mode::hrtf: return capsule.hrtf()->item.get().orientation;
    }
}

bool operator==(const capsule& a, const capsule& b) {
    return static_cast<const capsule::base_type&>(a) ==
                   static_cast<const capsule::base_type&>(b) &&
           a.get_name() == b.get_name() && a.get_mode() == b.get_mode();
}

bool operator!=(const capsule& a, const capsule& b) { return !(a == b); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
