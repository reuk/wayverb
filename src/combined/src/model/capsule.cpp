#include "combined/model/capsule.h"

namespace wayverb {
namespace combined {
namespace model {

capsule::capsule() { connect(microphone, hrtf); }

void capsule::swap(capsule& other) noexcept {
    using std::swap;
    swap(microphone, other.microphone);
    swap(hrtf, other.hrtf);
    swap(name_, other.name_);
    swap(mode_, other.mode_);
}

capsule::capsule(const capsule& other)
        : microphone{other.microphone}
        , hrtf{other.hrtf}
        , name_{other.name_}
        , mode_{other.mode_} {
    connect(microphone, hrtf);        
}

capsule::capsule(capsule&& other) noexcept {
    swap(other);
    connect(microphone, hrtf);
}

capsule& capsule::operator=(const capsule& other) {
    auto copy{other};
    swap(copy);
    connect(microphone, hrtf);
    return *this;
}

capsule& capsule::operator=(capsule&& other) noexcept {
    swap(other);
    connect(microphone, hrtf);
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

}  // namespace model
}  // namespace combined
}  // namespace wayverb
