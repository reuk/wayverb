#pragma once

#include "glm/fwd.hpp"

class null_attenuator final {};

double attenuation(null_attenuator, const glm::vec3& );
