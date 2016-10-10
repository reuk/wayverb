#pragma once

#include "utilities/range.h"

#include <array>
#include <cmath>

namespace frequency_domain {

/// See antoni2010 equations 19 and 20

double lower_band_edge(double centre, double p, double P, size_t l);
double upper_band_edge(double centre, double p, double P, size_t l);

double band_edge_frequency(size_t band, size_t bands, range<double> r);
double band_centre_frequency(size_t band, size_t bands, range<double> r);

double band_edge_width(size_t band,
                       size_t bands,
                       range<double> r,
                       double overlap);

/// Holds parameters for a frequency-domain crossover filter.
/// 'edge' is a frequency (Hz or normalised)
/// 'width' is the width of the crossover at this frequency
struct edge_and_width final {
    double edge;
    double width;
};

edge_and_width band_edge_and_width(size_t band,
                                   size_t bands,
                                   range<double> r,
                                   double overlap);

template <size_t... Ix>
auto band_edges_and_widths(range<double> r,
                           double overlap,
                           std::index_sequence<Ix...>) {
    return std::array<edge_and_width, sizeof...(Ix)>{
            {band_edge_and_width(Ix, sizeof...(Ix) - 1, r, overlap)...}};
}

template <size_t bands>
auto band_edges_and_widths(range<double> r, double overlap) {
    return band_edges_and_widths(
            r, overlap, std::make_index_sequence<bands + 1>{});
}

/// frequency: normalized frequency, from 0 to 0.5
/// lower: the normalized lower band edge frequency of this band
/// lower_edge_width: half the absolute width of the lower crossover
/// upper: the normalized upper band edge frequency of this band
/// upper_edge_width: half the absolute width of the upper crossover
/// l: the slope (0 is shallow, higher is steeper)
double compute_bandpass_magnitude(double frequency,
                                  edge_and_width lower,
                                  edge_and_width upper,
                                  size_t l = 0);

double compute_lopass_magnitude(double frequency,
                                edge_and_width cutoff,
                                size_t l = 0);

double compute_hipass_magnitude(double frequency,
                                edge_and_width cutoff,
                                size_t l = 0);

}  // namespace frequency_domain
