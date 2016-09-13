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

/// r = distance at which the geometric sound source has intensity 1W/m^2
/// sr = waveguide mesh sampling rate
inline double rectilinear_calibration_factor(double r,
                                             double sr,
                                             double speed_of_sound) {
    const auto courant{1 / std::sqrt(3.0)};
    const auto x{speed_of_sound / (courant * sr)};
    return r / (x * 0.3405);
}

inline double rectilinear_calibration_factor(double sr, double speed_of_sound) {
    return rectilinear_calibration_factor(
            distance_for_unit_intensity(1), sr, speed_of_sound);
}

}  // namespace waveguide
