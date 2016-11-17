#include "combined/model/source.h"

#include "utilities/map_to_vector.h"

namespace wayverb {
namespace combined {
namespace model {

source::source(const core::geo::box& aabb)
        : base_type{constrained_point{aabb}, hover_state_t{}}
        , name_{"new source"} {}

void source::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

std::string source::get_name() const { return name_; }

////////////////////////////////////////////////////////////////////////////////

sources::sources(const core::geo::box& aabb)
        : base_type{vector<source, 1>{source{aabb}}}
        , aabb_{aabb} {}

const shared_value<source>& sources::operator[](size_t index) const {
    return (*data())[index];
}
shared_value<source>& sources::operator[](size_t index) {
    return (*data())[index];
}

size_t sources::size() const { return data()->size(); }
bool sources::empty() const { return data()->empty(); }

void sources::clear() { data()->clear(); }

bool sources::can_erase() const { return data()->can_erase(); }

const shared_value<vector<source, 1>>& sources::data() const {
    return get<0>();
}
shared_value<vector<source, 1>>& sources::data() { return get<0>(); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
