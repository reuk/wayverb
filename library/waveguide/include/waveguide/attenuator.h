#pragma once

#include "waveguide/postprocessor/directional_receiver.h"

#include "utilities/map_to_vector.h"

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

/*
template <typename Method, typename It>
auto attenuate(const Method& method, float Z, It begin, It end) {
    //  TODO Filter with diffuse-field-response filter here.
    //  Make sure to use zero-phase filtering.
    return map_to_vector(
            begin, end, [&](const auto& i) { return attenuate(method, Z, i); });
}
*/

template <typename Method>
struct attenuate_mapper final {
    Method method;
    float Z;

    template <typename T>
    auto operator()(const T& t) const {
        return attenuate(method, Z, t);
    }
};

template <typename Method>
auto make_attenuate_mapper(Method method, float Z) {
    if (Z < 300 || 500 <= Z) {
        throw std::runtime_error{"acoustic impedance outside expected range"};
    }
    return attenuate_mapper<Method>{std::move(method), Z};
}


template <typename It, typename Method>
auto make_attenuator_iterator(It it, const Method& method, float Z) {
    return make_mapping_iterator_adapter(std::move(it),
                                         make_attenuate_mapper(method, Z));
}

}  // namespace waveguide
