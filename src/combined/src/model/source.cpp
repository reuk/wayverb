#include "combined/model/source.h"

#include "utilities/map_to_vector.h"

namespace wayverb {
namespace combined {
namespace model {

source::source(std::string name, glm::vec3 position)
        : base_type{hover_state_t{}}
        , name_{std::move(name)}
        , position_{std::move(position)} {}

void source::set_name(std::string name) {
    name_ = std::move(name);
    notify();
}

std::string source::get_name() const { return name_; }

void source::set_position(const glm::vec3& position) {
    position_ = position;
    notify();
}

glm::vec3 source::get_position() const { return position_; }

bool operator==(const source& a, const source& b) {
    return a.get_name() == b.get_name() && a.get_position() == b.get_position();
}

bool operator!=(const source& a, const source& b) {
    return !(a==b);
}

}  // namespace model
}  // namespace combined
}  // namespace wayverb
