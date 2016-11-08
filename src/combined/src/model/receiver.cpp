#include "combined/model/receiver.h"

#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

receiver::receiver(core::geo::box bounds)
        : bounds_{std::move(bounds)}
        , position_{centre(bounds_)} {
    connect(capsules);
}

void receiver::swap(receiver& other) noexcept {
    using std::swap;
    swap(capsules, other.capsules);
    swap(bounds_, other.bounds_);
    swap(name_, other.name_);
    swap(orientation_, other.orientation_);
}

receiver::receiver(const receiver& other)
        : capsules{other.capsules}
        , bounds_{other.bounds_}
        , name_{other.name_}
        , position_{other.position_}
        , orientation_{other.orientation_} {
    connect(capsules);
}

receiver::receiver(receiver&& other) noexcept {
    swap(other);
    connect(capsules);
}

receiver& receiver::operator=(const receiver& other) {
    auto copy{other};
    swap(copy);
    connect(capsules);
    return *this;
}

receiver& receiver::operator=(receiver&& other) noexcept {
    swap(other);
    connect(capsules);
    return *this;
}

void receiver::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

std::string receiver::get_name() const { return name_; }

void receiver::set_position(const glm::vec3& position) {
    position_ = clamp(position, bounds_);
    notify();
}

glm::vec3 receiver::get_position() const { return position_; }

void receiver::set_orientation(float azimuth, float elevation) {
    orientation_.set_pointing(
            compute_pointing(core::az_el{azimuth, elevation}));
    notify();
}

core::orientable receiver::get_orientation() const { return orientation_; }

////////////////////////////////////////////////////////////////////////////////

receivers::receivers(core::geo::box aabb)
        : aabb_{std::move(aabb)} {
    insert(0, receiver{aabb_});
    connect(receivers_);
}

void receivers::swap(receivers& other) noexcept {
    using std::swap;
    swap(aabb_, other.aabb_);
    swap(receivers_, other.receivers_);
}

receivers::receivers(const receivers& other)
        : aabb_{other.aabb_}
        , receivers_{other.receivers_} {
    connect(receivers_);
}

receivers::receivers(receivers&& other) noexcept {
    swap(other);
    connect(receivers_);
}

receivers& receivers::operator=(const receivers& other) {
    auto copy{other};
    swap(copy);
    connect(receivers_);
    return *this;
}

receivers& receivers::operator=(receivers&& other) noexcept {
    swap(other);
    connect(receivers_);
    return *this;
}

const receiver& receivers::operator[](size_t index) const {
    return receivers_[index];
}
receiver& receivers::operator[](size_t index) { return receivers_[index]; }

void receivers::insert(size_t index, receiver t) {
    receivers_.insert(receivers_.begin() + index, std::move(t));
}

void receivers::erase(size_t index) {
    if (1 < receivers_.size()) {
        receivers_.erase(receivers_.begin() + index);
    }
}

size_t receivers::size() const { return receivers_.size(); }

bool receivers::empty() const { return receivers_.empty(); }

void receivers::clear() { receivers_.clear(); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
