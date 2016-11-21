#include "combined/model/receiver.h"

#include "core/az_el.h"
#include "utilities/map_to_vector.h"

namespace wayverb {
namespace combined {
namespace model {

receiver::receiver(std::string name,
                   glm::vec3 position,
                   core::orientation orientation)
        : base_type{vector<capsule>{}, hover_state_t{}}
        , name_{std::move(name)}
        , position_{std::move(position)}
        , orientation_{std::move(orientation)} {}

void receiver::swap(receiver& other) noexcept {
    using std::swap;
    swap(name_, other.name_);
    swap(position_, other.position_);
    swap(orientation_, other.orientation_);
}

receiver& receiver::operator=(receiver other) {
    base_type::operator=(other);
    swap(other);
    notify();
    return *this;
}

void receiver::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

std::string receiver::get_name() const { return name_; }

void receiver::set_orientation(const core::orientation& orientation) {
    orientation_ = orientation;
    notify();
}

core::orientation receiver::get_orientation() const { return orientation_; }

void receiver::set_position(const glm::vec3& p) {
    position_ = p;
    notify();
}

glm::vec3 receiver::get_position() const { return position_; }

bool operator==(const receiver& a, const receiver& b) {
    return a.get_name() == b.get_name() &&
           a.get_position() == b.get_position() &&
           a.get_orientation() == b.get_orientation();
}

bool operator!=(const receiver& a, const receiver& b) { return !(a == b); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
