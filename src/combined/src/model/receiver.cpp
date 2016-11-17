#include "combined/model/receiver.h"

#include "core/az_el.h"
#include "utilities/map_to_vector.h"

namespace wayverb {
namespace combined {
namespace model {

receiver::receiver(core::geo::box bounds)
        : base_type{constrained_point{bounds},
                    vector<capsule, 1>{},
                    hover_state_t{}} {}

void receiver::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

std::string receiver::get_name() const { return name_; }

void receiver::set_orientation(float azimuth, float elevation) {
    orientation_.set_pointing(
            compute_pointing(core::az_el{azimuth, elevation}));
    notify();
}

core::orientation receiver::get_orientation() const { return orientation_; }

////////////////////////////////////////////////////////////////////////////////

receivers::receivers(const core::geo::box& aabb)
        : base_type{vector<receiver, 1>{receiver{aabb}}}
        , aabb_{aabb} {}

const shared_value<receiver>& receivers::operator[](size_t index) const {
    return (*data())[index];
}
shared_value<receiver>& receivers::operator[](size_t index) {
    return (*data())[index];
}

size_t receivers::size() const { return data()->size(); }
bool receivers::empty() const { return data()->empty(); }

void receivers::clear() { data()->clear(); }

bool receivers::can_erase() const { return data()->can_erase(); }

shared_value<vector<receiver, 1>>& receivers::data() { return get<0>(); }
const shared_value<vector<receiver, 1>>& receivers::data() const {
    return get<0>();
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
