#pragma once

//  pressure - N / m^2
//  force per unit area
//
//  intensity - mean energy flow 
//      energy transported per second through a unit area
//
//  If several pressure signals are present, their total level is the sum of the
//      individual pressures.
//
//  For cohrent, in-phase signals the pressure-time functions are added,
//      doubling the pressure (a level change of 6dB)
//
//  If they're in antiphase, the signals cancel (minus infinity level)
//
//  If they're incorherent, you can sum intensities directly.


namespace raytracer {

/// vorlander book, equation 11.37
double reflection_factor_magnitude(double absorption_coefficient) {
    return std::sqrt(1 - absorption_coefficient);
}



}//namespace raytracer
