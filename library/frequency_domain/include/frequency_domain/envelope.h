#pragma once

#include "utilities/range.h"

#include <array>
#include <cmath>

namespace frequency_domain {

double max_width_factor(range<double> r, double step);

/// See antoni2010 equations 19 and 20

double lower_band_edge(double centre, double p, double P, size_t l);
double upper_band_edge(double centre, double p, double P, size_t l);

double band_edge_frequency(size_t band, size_t bands, range<double> r);
double band_centre_frequency(size_t band, size_t bands, range<double> r);

/// Holds parameters for a frequency-domain crossover filter.
/// 'edge' is a frequency (Hz or normalised)
/// 'width' is the width of the crossover at this frequency
struct edge_and_width final {
    double edge;
    double width;
};

constexpr auto edge(const edge_and_width& e) {
    return e.edge;
}

constexpr auto make_edge_with_width_factor(double edge, double width_factor) {
    return edge_and_width{edge, edge * width_factor};
}

template <size_t... Ix>
auto band_edges_and_widths(range<double> r,
                           double max_width_factor,
                           std::index_sequence<Ix...>) {
    return std::array<edge_and_width, sizeof...(Ix)>{
            {make_edge_with_width_factor(
                    band_edge_frequency(Ix, sizeof...(Ix) - 1, r),
                    max_width_factor)...}};
}

template <size_t bands>
auto band_edges_and_widths(range<double> r, double overlap) {
    if (overlap < 0 || 1 < overlap) {
        throw std::runtime_error{"overlap is outside valid range [0, 1)"};
    }
    const auto width_factor{max_width_factor(r, 1.0 / bands) * overlap};
    return band_edges_and_widths(
            r, width_factor, std::make_index_sequence<bands + 1>{});
}

template <typename T, size_t... Ix>
auto bandwidths(const std::array<T, sizeof...(Ix) + 1>& edges,
                std::index_sequence<Ix...>) {
    using value_type = decltype(edge(edges[1]) - edge(edges[0]));
    return std::array<value_type, sizeof...(Ix)>{
            {edge(edges[Ix + 1]) - edge(edges[Ix])...}};
}

template <typename T, size_t edges>
auto bandwidths(const std::array<T, edges>& e) {
    return bandwidths(e, std::make_index_sequence<edges - 1>{});
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
