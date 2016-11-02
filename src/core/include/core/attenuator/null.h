#pragma once

#include "glm/fwd.hpp"

namespace core {
namespace attenuator {

class null final {};

constexpr auto attenuation(const null&, const glm::vec3&) { return 1.0; }

}  // namespace attenuator
}  // namespace core
