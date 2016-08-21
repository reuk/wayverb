#pragma once

#include "common/aligned/vector.h"
#include "common/orientable.h"

#include <vector>

namespace model {

struct Pointer {
    enum class Mode { spherical, look_at };

    glm::vec3 get_pointing(const glm::vec3& position) const;

    Mode mode{Mode::spherical};
    AzEl spherical{};
    glm::vec3 look_at{0};
};

//----------------------------------------------------------------------------//

struct Microphone {
    Pointer pointer{};
    float shape{0};
};

//----------------------------------------------------------------------------//

struct ReceiverSettings {
    enum class Mode { microphones, hrtf };

    aligned::vector<glm::vec3> get_pointing() const;

    glm::vec3 position{0, 0, 0};
    Mode mode{Mode::microphones};
    aligned::vector<Microphone> microphones{Microphone{}};
    Pointer hrtf{};
};

}  // namespace model
