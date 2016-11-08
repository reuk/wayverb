#include "combined/model/source.h"

namespace wayverb {
namespace combined {
namespace model {

source::source(core::geo::box bounds)
        : bounds_{std::move(bounds)}
        , name_{"new source"}
        , position_{centre(bounds_)} {}

void source::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

std::string source::get_name() const { return name_; }

void source::set_position(const glm::vec3& position) {
    position_ = clamp(position, bounds_);
    notify();
}

glm::vec3 source::get_position() const { return position_; }

}  // namespace model
}  // namespace combined
}  // namespace wayverb
