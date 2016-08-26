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

//----------------------------------------------------------------------------//
//
//  kutruff2009 p.328
//  -----------------
//
//  energy results as a function of frequency band i and time interval k:
//      E(fi, tk)
//      where k is not the final samplerate, but rather a period of a few ms
//
//  E(fi, tk) approximates a short-time power spectrum valid for time interval k
//
//  Smooth the spectrum, to get Ek(f) i.e. the smoothed energy at frequency f
//  and time k
//
//  Invent a phase spectrum...
//      apply the hilbert transform to the natural log of Ek(f) to find the
//      minimum phase function
//
//  heinz1993
//  ---------
//  
//  Take a sequence of dirac pulses which are poisson distributed along the time
//  axis.
//
//  Filter these pulses in the time domain by convolving each pulse of the 
//  poisson sequence with the inverse fourier transform of the square-rooted,
//  smoothed and interpolated power spectrum of the corresponding time interval.
//
//----------------------------------------------------------------------------//

aligned::vector<float> energy_histogram_to_impulse_response(
        const aligned::vector<volume_type>& histogram,
        double period,
        double sample_rate);
