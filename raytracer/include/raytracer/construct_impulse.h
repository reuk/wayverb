#pragma once

#include "raytracer/cl_structs.h"

VolumeType air_attenuation_for_distance(float distance);
float power_attenuation_for_distance(float distance);
VolumeType attenuation_for_distance(float distance);
Impulse construct_impulse(const VolumeType& volume,
                          const glm::vec3& source,
                          float distance);
