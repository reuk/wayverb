#include "raytracer/construct_impulse.h"

#include "common/cl_traits.h"
#include "common/conversions.h"
#include "common/stl_wrappers.h"

VolumeType air_attenuation_for_distance(float distance) {
    auto ret = air_coefficient * distance;
    for (auto& i : ret.s) {
        i = std::pow(M_E, i);
    }
    return ret;
}

float power_attenuation_for_distance(float distance) {
    return 1 / (4 * M_PI * distance * distance);
}

VolumeType attenuation_for_distance(float distance) {
    const auto air   = air_attenuation_for_distance(distance);
    const auto power = power_attenuation_for_distance(distance);
    return air * power;
}

Impulse construct_impulse(const VolumeType& volume,
                          const glm::vec3& source,
                          float distance) {
    return Impulse{volume * attenuation_for_distance(distance),
                   to_cl_float3(source),
                   static_cast<cl_float>(distance / SPEED_OF_SOUND)};
}
