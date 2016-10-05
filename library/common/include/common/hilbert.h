#pragma once

#include "common/aligned/vector.h"

#include <complex>

aligned::vector<std::complex<float>> make_complex(
        const aligned::vector<float>& x);

aligned::vector<std::complex<float>> ifft(
        const aligned::vector<std::complex<float>>& x);
aligned::vector<std::complex<float>> fft(
        const aligned::vector<std::complex<float>>& x);

/// Given a complex spectrum, calculate a corresponding minimum-phase version.
/// See: https://ccrma.stanford.edu/~jos/fp/Matlab_listing_mps_m.html
aligned::vector<std::complex<float>> mps(
        aligned::vector<std::complex<float>> spectrum);
