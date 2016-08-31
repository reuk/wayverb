#pragma once

#include "raytracer/cl/structs.h"

#include "glm/fwd.hpp"

namespace raytracer {

float power_attenuation_for_distance(float distance);
impulse construct_impulse(const volume_type& volume,
                          const glm::vec3& source,
                          double distance,
                          double speed_of_sound);

}  // namespace raytracer
