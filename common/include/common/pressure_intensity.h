#pragma once

#include <cmath>

template <typename T, typename U>
inline T pressure_to_intensity(T pressure, U Z) {
    return pressure * pressure / Z;
}

template <typename T, typename U>
inline T intensity_to_pressure(T intensity, U Z) {
    using std::copysign;
    using std::sqrt;
    using std::abs;
    return copysign(sqrt(abs(intensity * Z)), intensity);
}
