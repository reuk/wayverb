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

////////////////////////////////////////////////////////////////////////////////

capsules::capsules() {
    insert(0, capsule{});
    connect(capsules_);
}

void capsules::swap(capsules& other) noexcept {
    using std::swap;
    swap(capsules_, other.capsules_);
}

capsules::capsules(const capsules& other)
        : capsules_{other.capsules_} {
    connect(capsules_);
}

capsules::capsules(capsules&& other) noexcept {
    swap(other);
    connect(capsules_);
}

capsules& capsules::operator=(const capsules& other) {
    auto copy{other};
    swap(copy);
    connect(capsules_);
    return *this;
}

capsules& capsules::operator=(capsules&& other) noexcept {
    swap(other);
    connect(capsules_);
    return *this;
}

const capsule& capsules::operator[](size_t index) const {
    return capsules_[index];
}
capsule& capsules::operator[](size_t index) { return capsules_[index]; }

void capsules::insert(size_t index, capsule t) {
    capsules_.insert(capsules_.begin() + index, std::move(t));
}

void capsules::erase(size_t index) {
    if (1 < capsules_.size()) {
        capsules_.erase(capsules_.begin() + index);
    }
}

size_t capsules::size() const { return capsules_.size(); }

bool capsules::empty() const { return capsules_.empty(); }

void capsules::clear() { capsules_.clear(); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
