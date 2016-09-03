#pragma once

#include "common/aligned/vector.h"
#include "common/cl/scene_structs.h"
#include "common/stl_wrappers.h"
#include <cmath>

//  Z: specific acoustic impedance, c. 400
//  ambient density: acoustic impedance / speed of sound

template <typename T>
inline T pressure_to_intensity(T pressure, T Z) {
    return pressure * pressure / Z;
}

template <typename T>
T intensity_to_pressure(T intensity, T Z) {
    return std::copysign(std::sqrt(std::abs(intensity) * Z), intensity);
}

inline volume_type intensity_to_pressure(const volume_type& intensity,
                                         float Z) {
    volume_type ret;
    proc::transform(intensity.s, ret.s, [=](auto i) {
        return intensity_to_pressure(i, Z);
    });
    return ret;
}
