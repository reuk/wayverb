#pragma once

#include <cmath>

/// See siltanen2013

namespace waveguide {
/// Given a source strength, calculate the distance at which the source produces
/// intensity 1W/m^2.
template <typename T>
inline auto distance_for_unit_intensity(T strength) {
    //  Intensity for distance is given by 1 / (4 * pi * distance * distance)
    using std::sqrt;
    return sqrt(strength / (4 * M_PI));
}

inline double rectilinear_calibration_factor(double grid_spacing) {
    return distance_for_unit_intensity(1) / (0.3405 * grid_spacing);
}

}  // namespace waveguide
