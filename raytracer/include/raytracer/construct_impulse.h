#pragma once

#include "raytracer/cl/structs.h"

#include "glm/fwd.hpp"

namespace raytracer {

volume_type air_attenuation_for_distance(float distance);
float power_attenuation_for_distance(float distance);
volume_type attenuation_for_distance(float distance);
impulse construct_impulse(const volume_type& volume,
                          const glm::vec3& source,
                          float distance);

}  // namespace raytracer
