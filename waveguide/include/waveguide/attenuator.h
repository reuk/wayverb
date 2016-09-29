#pragma once

#include "waveguide/postprocessor/directional_receiver.h"

namespace waveguide {

template <typename Method>
auto attenuate(const Method& method,
               const postprocessor::directional_receiver::output& i) {
    using std::sqrt;
    using std::copysign;
    using std::pow;
    return copysign(sqrt(glm::length(i.intensity) *
                         pow(attenuation(method, i.intensity), 2.0f)),
                    i.pressure);
}

template <typename Method, typename It>
auto attenuate(const Method& method, It begin, It end) {
    //  TODO Filter with diffuse-field-response filter here.
    //  Make sure to use zero-phase filtering.
    return map_to_vector(
            begin, end, [&](const auto& i) { return attenuate(method, i); });
}

}  // namespace waveguide
