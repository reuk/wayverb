#pragma once

#include "core/filters_common.h"

#include "utilities/aligned/vector.h"

namespace wayverb {
namespace waveguide {

/// See sheaffer2014 and the source-modelling-toolbox.
/// paper: physical and numerical constraints in source modeling for finite
/// difference simulation of room acoustics
/// code:  http://code.soundsoftware.ac.uk/projects/smt/

template <typename T>
constexpr auto factdbl(T t) {
    double out{1};
    for (auto i = t; i >= 1; i -= 2) {
        out *= i;
    }
    return out;
}

struct offset_signal final {
    util::aligned::vector<double> signal;
    size_t offset;
};

/// Design a maxflat lowpass pulse.
///
/// f0:     normalised cutoff frequency
/// N:      filter order
/// A:      pulse peak amplitude
/// hLen:   full, padded length
offset_signal maxflat(double f0, uint32_t N, double A, uint32_t h_len);

/// Compute mech sphere corrective output gain.
double compute_g0(double acoustic_impedance,
                  double speed_of_sound,
                  double sample_rate,
                  double radius);

/// Design a mechanical filter simulating a small pulsating sphere.
///
/// M:      mass of sphere
/// f0:     normalised low resonance of mechanical system
/// Q:      Q of mechanical system
/// T:      temporal sample period
core::filter::biquad::coefficients mech_sphere(double M,
                                               double f0,
                                               double Q,
                                               double T);

/// Create an input signal from scratch.
///
/// According to sheaffer terminology, the first 'pulse shaping' filter is
/// a 'maxflat' FIR filter.
/// This filter's cutoff is placed AT the waveguide nyquist (i.e. 0.25 fs),
/// and the filter uses 64 taps to give a steep rolloff.
///
/// The initial kernel is filtered by an IIR mechanical filter, which simulates
/// a pulsating sphere.
/// The parameters for this filter are arbitrary at the moment.
///
/// Finally, the output from the first two filters is filtered by a 'injection'
/// IIR filter.
/// This filter approximates the time derivative of the previous filters.
///
/// *NOTE*
/// This input signal should be used as a soft, not a transparent source.
///
/// TODO find correct mass for sphere so that level correction still works
/// properly.
offset_signal design_pcs_source(size_t length,
                                double acoustic_impedance,
                                double speed_of_sound,
                                double sample_rate,
                                double radius,
                                double sphere_mass,
                                double low_cutoff_hz,
                                double low_q);

}  // namespace waveguide
}  // namespace wayverb
