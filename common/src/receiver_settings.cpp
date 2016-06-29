#include "common/receiver_settings.h"

namespace model {

glm::vec3 Pointer::get_pointing(const glm::vec3& position) const {
    switch (mode) {
        case Mode::spherical:
            return Orientable::compute_pointing(spherical);
        case Mode::look_at:
            return glm::normalize(look_at - position);
    }
}

} // namespace model
