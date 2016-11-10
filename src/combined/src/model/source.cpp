#include "combined/model/source.h"

namespace wayverb {
namespace combined {
namespace model {

source::source(const core::geo::box& aabb)
        : type{constrained_point{aabb}}
        , name_{"new source"} {}

void source::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

std::string source::get_name() const { return name_; }

////////////////////////////////////////////////////////////////////////////////

sources::sources(const core::geo::box& aabb)
        : aabb_{aabb} {}

const source& sources::operator[](size_t index) const { return data()[index]; }
source& sources::operator[](size_t index) { return data()[index]; }

size_t sources::size() const { return data().size(); }
bool sources::empty() const { return data().empty(); }

void sources::clear() { data().clear(); }

bool sources::can_erase() const { return data().can_erase(); }

vector<source, 1>& sources::data() { return get<0>(); }
const vector<source, 1>& sources::data() const { return get<0>(); }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
