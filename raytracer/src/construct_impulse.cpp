#include "raytracer/construct_impulse.h"

#include "common/cl_traits.h"
#include "common/config.h"
#include "common/conversions.h"
#include "common/stl_wrappers.h"

namespace raytracer {

volume_type air_attenuation_for_distance(float distance) {
    auto ret = air_coefficient * distance;
    for (auto& i : ret.s) {
        i = std::pow(M_E, i);
    }
    return ret;
}

float power_attenuation_for_distance(float distance) {
    return 1 / (4 * M_PI * distance * distance);
}

volume_type attenuation_for_distance(float distance) {
    const auto air   = air_attenuation_for_distance(distance);
    const auto power = power_attenuation_for_distance(distance);
    return air * power;
}

impulse construct_impulse(const volume_type& volume,
                          const glm::vec3& source,
                          float distance) {
    return impulse{volume * attenuation_for_distance(distance),
                   to_cl_float3(source),
                   static_cast<cl_float>(distance / speed_of_sound)};
}

}  // namespace raytracer
