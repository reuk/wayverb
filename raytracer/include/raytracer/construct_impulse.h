#pragma once

#include "raytracer/cl_structs.h"

/// Call binary operation u on pairs of elements from a and b, where a and b are
/// cl_floatx types.
template <typename T, typename U>
inline T elementwise(const T& a, const T& b, const U& u) {
    T ret;
    proc::transform(a.s, std::begin(b.s), std::begin(ret.s), u);
    return ret;
}

VolumeType air_attenuation_for_distance(float distance);
float power_attenuation_for_distance(float distance);
VolumeType attenuation_for_distance(float distance);
Impulse construct_impulse(const VolumeType& volume,
                          const glm::vec3& source,
                          float distance);
