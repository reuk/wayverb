#include "raytracer/construct_impulse.h"

#include "common/cl/traits.h"
#include "common/conversions.h"
#include "common/stl_wrappers.h"

namespace raytracer {

float power_attenuation_for_distance(float distance) {
    return 1 / (4 * M_PI * distance * distance);
}

impulse construct_impulse(const volume_type& volume,
                          const glm::vec3& source,
                          const glm::vec3& receiver,
                          double speed_of_sound) {
    const auto distance{glm::distance(source, receiver)};
    return impulse{volume * power_attenuation_for_distance(distance),
                   to_cl_float3(source),
                   static_cast<cl_float>(distance / speed_of_sound)};
}

}  // namespace raytracer
