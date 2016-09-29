#pragma once

#include "raytracer/cl/structs.h"

#include "common/map_to_vector.h"

#include "glm/glm.hpp"

namespace raytracer {

template <typename Method, size_t channels>
auto attenuate(const Method& method,
               const glm::vec3& position,
               const impulse<channels>& i) {
    return attenuated_impulse<channels>{
            i.volume * attenuation(method, to_vec3(i.position) - position),
            i.distance};
}

template <typename Method, typename It>
auto attenuate(const Method& method,
               const glm::vec3& position,
               It begin,
               It end) {
    return map_to_vector(begin, end, [&](const auto& impulse) {
        return attenuate(method, position, impulse);
    });
}

}  // namespace raytracer
