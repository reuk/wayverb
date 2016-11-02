#include "core/model/orientable.h"

namespace model {

glm::vec3 get_pointing(const az_el& azimuth_elevation,
                       const glm::vec3& position) {
    return compute_pointing(azimuth_elevation);
}

glm::vec3 get_pointing(const glm::vec3& look_at, const glm::vec3& position) {
    return glm::normalize(look_at - position);
}

glm::vec3 get_pointing(const orientable& o, const glm::vec3& position) {
    switch (o.mode) {
        case orientable::mode::spherical:
            return get_pointing(o.spherical, position);
        case orientable::mode::look_at:
            return get_pointing(o.look_at, position);
    }
}

}  // namespace model
