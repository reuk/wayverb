#pragma once

#include <complex>
#include <vector>

/// Given a complex spectrum, calculate a corresponding minimum-phase version.
/// See: https://ccrma.stanford.edu/~jos/fp/Matlab_listing_mps_m.html
std::vector<std::complex<float>> mps(std::vector<std::complex<float>> spectrum);
