#pragma once

#include "core/orientable.h"

#include "glm/glm.hpp"

namespace model {

glm::vec3 get_pointing(const az_el& az_el, const glm::vec3& position);
glm::vec3 get_pointing(const glm::vec3& look_at, const glm::vec3& position);

struct orientable final {
    enum class mode { spherical, look_at };
    mode mode{mode::spherical};
    az_el spherical;
    glm::vec3 look_at;
};

glm::vec3 get_pointing(const orientable& o, const glm::vec3& position);

}  // namespace model
