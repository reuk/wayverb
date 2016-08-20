#pragma once

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
