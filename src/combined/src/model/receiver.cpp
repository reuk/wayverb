#include "combined/model/receiver.h"

#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

receiver::receiver(core::geo::box bounds)
        : bounds_{std::move(bounds)}
        , position_{centre(bounds_)} {
    capsules_.emplace(0);
    connect(std::tie(capsules_));
}

void receiver::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

void receiver::set_position(const glm::vec3& position) {
    position_ = clamp(position, bounds_);
    notify();
}

void receiver::set_orientation(float azimuth, float elevation) {
    orientation_.set_pointing(
            compute_pointing(core::az_el{azimuth, elevation}));
    notify();
}

const capsule& receiver::get_capsule(size_t index) const {
    return capsules_[index];
}

capsule& receiver::get_capsule(size_t index) { return capsules_[index]; }

void receiver::add_capsule(size_t index) { capsules_.emplace(index); }

void receiver::remove_capsule(size_t index) {
    if (1 < capsules_.size()) {
        capsules_.erase(index);
    }
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
