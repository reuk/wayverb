#pragma once

#include "common/aligned/vector.h"
#include "common/cl/scene_structs.h"
#include "common/stl_wrappers.h"
#include <cmath>

//  Z: specific acoustic impedance, c. 400
//  ambient density: acoustic impedance / speed of sound

template <typename T, typename U>
inline T pressure_to_intensity(T pressure, U Z) {
    return pressure * pressure / Z;
}

template <typename T>
T intensity_to_pressure(T intensity, T Z) {
    using std::copysign;
    using std::sqrt;
    using std::abs;
    return copysign(sqrt(abs(intensity) * Z), intensity);
}
