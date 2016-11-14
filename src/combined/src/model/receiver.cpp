#include "combined/model/receiver.h"

#include "utilities/map_to_vector.h"
#include "core/az_el.h"

namespace wayverb {
namespace combined {
namespace model {

receiver::receiver(core::geo::box bounds)
        : type{constrained_point{bounds}, vector<capsule, 1>{}} {}

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

receiver::raw receiver::get_raw() const {
    struct capsule_get_raw final {
        auto operator()(const capsule& capsule) const {
            return capsule.get_raw();
        }
    };

    auto make_iterator = [](auto it) {
        return util::make_mapping_iterator_adapter(std::move(it),
                                                   capsule_get_raw{});
    };

    std::vector<capsule::raw> raw(make_iterator(std::begin(capsules())),
                                  make_iterator(std::end(capsules())));
    return {name_, get_orientation(), position().get(), std::move(raw)};
}

////////////////////////////////////////////////////////////////////////////////

receivers::receivers(const core::geo::box& aabb)
        : aabb_{aabb} {}

const receiver& receivers::operator[](size_t index) const {
    return data()[index];
}
receiver& receivers::operator[](size_t index) { return data()[index]; }

size_t receivers::size() const { return data().size(); }
bool receivers::empty() const { return data().empty(); }

void receivers::clear() { data().clear(); }

bool receivers::can_erase() const { return data().can_erase(); }

vector<receiver, 1>& receivers::data() { return get<0>(); }
const vector<receiver, 1>& receivers::data() const { return get<0>(); }

std::vector<receiver::raw> receivers::get_raw() const {
    struct receiver_get_raw final {
        auto operator()(const receiver& receiver) const {
            return receiver.get_raw();
        }
    };

    auto make_iterator = [](auto it) {
        return util::make_mapping_iterator_adapter(std::move(it),
                                                   receiver_get_raw{});
    };

    return std::vector<receiver::raw>(make_iterator(cbegin()),
                                      make_iterator(cend()));
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
