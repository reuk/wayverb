#pragma once

#include <cmath>
#include <iostream>

/// \file calibration
/// Defines level-matching methods for waveguide and raytracer outputs.
/// See siltanen2013.

namespace wayverb {
namespace waveguide {

/// Given a source strength, calculate the distance at which the source produces
/// intensity 1W/m^2.
inline double distance_for_unit_intensity() {
    //  Intensity for distance is given by 1 / (4 * pi * distance * distance)
    using std::sqrt;
    return sqrt(1.0 / (4 * M_PI));
}

inline double distance_for_unit_pressure(double acoustic_impedance) {
    using std::sqrt;
    return sqrt(acoustic_impedance / (4 * M_PI));
}

template <typename T, typename U>
inline auto rectilinear_calibration_factor(T grid_spacing,
                                           U acoustic_impedance) {
    const auto ret = distance_for_unit_pressure(acoustic_impedance) /
                     (0.3405 * grid_spacing);
    std::cout << "grid_spacing: " << grid_spacing << '\n';
    std::cout << "rectilinear_calibration_factor: " << ret << '\n';
    return ret;
}

}  // namespace waveguide
}  // namespace wayverb
