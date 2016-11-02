#pragma once

#include "glm/glm.hpp"

namespace core {
namespace model {

struct parameters final {
    glm::vec3 source;
    glm::vec3 receiver;
    double speed_of_sound{340.0};
    double acoustic_impedance{400.0};
};

constexpr auto get_ambient_density(const parameters& s) {
    return s.acoustic_impedance / s.speed_of_sound;
}

}  // namespace model
}  // namespace core
