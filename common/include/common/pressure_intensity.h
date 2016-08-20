#pragma once

#include <cmath>

template <typename T>
inline T pressure_to_intensity(T pressure, T Z = T{400}) {
    return pressure * pressure / Z;
}

template <typename T>
T intensity_to_pressure(T intensity, T Z = T{400}) {
    return std::copysign(std::sqrt(std::abs(intensity) * Z), intensity);
}
