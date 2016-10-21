#pragma once

#include "raytracer/cl/structs.h"

#include "common/attenuator/hrtf.h"
#include "common/attenuator/microphone.h"
#include "common/attenuator/null.h"
#include "common/conversions.h"

#include "utilities/map_to_vector.h"

#include "glm/glm.hpp"

namespace raytracer {

template <size_t channels>
auto attenuate(const attenuator::null& null,
               const glm::vec3& position,
               const impulse<channels>& i) {
    return make_attenuated_impulse(i.volume, i.distance);
}

/// For a microphone, we just look up the direction of the impulse from the
/// receiver and scale the volume appropriately.
template <size_t channels>
auto attenuate(const attenuator::microphone& mic,
               const glm::vec3& position,
               const impulse<channels>& i) {
    const auto dir = to_vec3(i.position) - position;
    const auto att = attenuation(mic, dir);
    return make_attenuated_impulse(i.volume * att, i.distance);
}

/// For the hrtf, we adjust the receiver positions a tiny bit depending on
/// whether the channel is left or right, which should introduce some reasonably
/// convincing interchannel time difference.
template <size_t channels>
auto attenuate(const attenuator::hrtf& hrtf,
               const glm::vec3& position,
               const impulse<channels>& i) {
    const auto adjusted_listener_position = get_ear_position(hrtf, position);

    const auto impulse_position = to_vec3(i.position);

    const auto dir = impulse_position - adjusted_listener_position;
    const auto att = attenuation(hrtf, dir);

    return make_attenuated_impulse(
            i.volume * att,
            glm::distance(impulse_position, adjusted_listener_position));
}

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
auto make_attenuator_iterator(It it, Method method, const glm::vec3& position) {
    return make_mapping_iterator_adapter(
            std::move(it), make_attenuate_mapper(std::move(method), position));
}

}  // namespace raytracer
