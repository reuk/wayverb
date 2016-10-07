#pragma once

#include "utilities/range.h"

#include <array>
#include <cmath>

namespace frequency_domain {

/// See antoni2010 equations 19 and 20

double lower_band_edge(double centre, double p, double P, size_t l);
double upper_band_edge(double centre, double p, double P, size_t l);

double band_edge_frequency(int band, size_t bands, range<double> r);

template <size_t bands>
std::array<double, bands + 1> band_edge_frequencies(range<double> r) {
    std::array<double, bands + 1> ret;
    for (auto i{0u}; i != bands + 1; ++i) {
        ret[i] = band_edge_frequency(i, bands, r);
    }
    return ret;
}

template <size_t bands>
std::array<double, bands + 1> band_centre_frequencies(range<double> r) {
    std::array<double, bands + 1> ret;
    for (auto i{0ul}; i != ret.size(); ++i) {
        ret[i] = band_edge_frequency(i * 2 + 1, bands * 2, r);
    }
    return ret;
}

template <size_t bands>
std::array<double, bands + 1> band_edge_widths(range<double> r,
                                               double overlap) {
    std::array<double, bands + 1> ret;
    for (int i{0}; i != ret.size(); ++i) {
        ret[i] = std::abs((band_edge_frequency(i * 2 + 0, bands * 2, r) -
                           band_edge_frequency(i * 2 + 1, bands * 2, r)) *
                          overlap);
    }
    return ret;
}

/// frequency: normalized frequency, from 0 to 0.5
/// lower: the normalized lower band edge frequency of this band
/// lower_edge_width: half the absolute width of the lower crossover
/// upper: the normalized upper band edge frequency of this band
/// upper_edge_width: half the absolute width of the upper crossover
/// l: the slope (0 is shallow, higher is steeper)
double compute_bandpass_magnitude(double frequency,
                                  range<double> r,
                                  double lower_edge_width,
                                  double upper_edge_width,
                                  size_t l);

double compute_lopass_magnitude(double frequency,
                                double cutoff,
                                double width,
                                size_t l);

double compute_hipass_magnitude(double frequency,
                                double cutoff,
                                double width,
                                size_t l);

}  // namespace frequency_domain
