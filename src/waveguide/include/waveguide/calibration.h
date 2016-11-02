#pragma once

#include <cmath>

/// See siltanen2013

namespace wayverb {
namespace waveguide {
/// Given a source strength, calculate the distance at which the source produces
/// intensity 1W/m^2.
template <typename T>
inline auto distance_for_unit_intensity(T strength) {
    //  Intensity for distance is given by 1 / (4 * pi * distance * distance)
    using std::sqrt;
    return sqrt(strength / (4 * M_PI));
}

template <typename T, typename U>
inline auto distance_for_unit_pressure(T strength, U acoustic_impedance) {
    using std::sqrt;
    return sqrt((strength * acoustic_impedance) / (4 * M_PI));
}

template <typename T, typename U>
inline auto rectilinear_calibration_factor(T grid_spacing,
                                           U acoustic_impedance) {
    return distance_for_unit_pressure(1.0, acoustic_impedance) /
           (0.3405 * grid_spacing);
}

}  // namespace waveguide
}  // namespace wayverb
