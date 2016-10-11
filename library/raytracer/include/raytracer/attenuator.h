#pragma once

#include "raytracer/cl/structs.h"

#include "common/conversions.h"

#include "utilities/map_to_vector.h"

#include "glm/glm.hpp"

namespace raytracer {

template <typename Method, size_t channels>
auto attenuate(const Method& method,
               const glm::vec3& position,
               const impulse<channels>& i) {
    const auto dir{to_vec3(i.position) - position};
    const auto att{attenuation(method, dir)};
    return make_attenuated_impulse(i.volume * att, i.distance);
}

/*
template <typename Method, typename It>
auto attenuate(const Method& method,
               const glm::vec3& position,
               It begin,
               It end) {
    return map_to_vector(begin, end, [&](const auto& impulse) {
        return attenuate(method, position, impulse);
    });
}
*/

template <typename Method>
struct attenuate_mapper final {
    Method method;
    glm::vec3 position;

    template <typename T>
    auto operator()(const T& t) const {
        return attenuate(method, position, t);
    }
};

template <typename Method>
auto make_attenuate_mapper(Method method, const glm::vec3& position) {
    return attenuate_mapper<Method>{std::move(method), position};
}

template <typename It, typename Method>
auto make_attenuator_iterator(It it,
                              const Method& method,
                              const glm::vec3& position) {
    return make_mapping_iterator_adapter(
            std::move(it), make_attenuate_mapper(method, position));
}

}  // namespace raytracer
