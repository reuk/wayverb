#pragma once

#include "waveguide/postprocessor/directional_receiver.h"

#include "common/map_to_vector.h"

namespace waveguide {

template <typename Method>
auto attenuate(const Method& method,
               float Z,
               const postprocessor::directional_receiver::output& i) {
    using std::sqrt;
    using std::copysign;
    using std::pow;
    const auto att{attenuation(method, -i.intensity)};
    const auto intensity{glm::length(i.intensity) * pow(att, 2.0f)};
    //  return copysign(sqrt(intensity), i.pressure);   //  haci2010 method
    return copysign(sqrt(intensity * Z), i.pressure);   //  scaled method
    //  return att * i.pressure;
}

template <typename Method, typename It>
auto attenuate(const Method& method, float Z, It begin, It end) {
    //  TODO Filter with diffuse-field-response filter here.
    //  Make sure to use zero-phase filtering.
    return map_to_vector(
            begin, end, [&](const auto& i) { return attenuate(method, Z, i); });
}

}  // namespace waveguide
