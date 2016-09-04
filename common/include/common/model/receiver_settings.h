#pragma once

#include "common/aligned/vector.h"
#include "common/orientable.h"

#include <vector>

namespace model {

struct orientable final {
    enum class mode { spherical, look_at };

    mode mode{mode::spherical};
    az_el spherical{};
    glm::vec3 look_at{0};
};

glm::vec3 get_pointing(const orientable& u, const glm::vec3& position);

//----------------------------------------------------------------------------//

struct microphone final {
    orientable orientable{};
    float shape{0};
};

//----------------------------------------------------------------------------//

struct receiver_settings final {
    enum class mode { microphones, hrtf };

    glm::vec3 position{0, 0, 0};
    mode mode{mode::microphones};
    aligned::vector<microphone> microphones{microphone{}};
    orientable hrtf{};
};

aligned::vector<glm::vec3> get_pointing(const receiver_settings& u);

}  // namespace model
