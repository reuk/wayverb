#pragma once

#include "utilities/range.h"

#include <array>
#include <cmath>

namespace frequency_domain {

double max_width_factor(util::range<double> r, double step);
double width_factor(util::range<double> r, size_t bands, double overlap);

/// See antoni2010 equations 19 and 20

double lower_band_edge(double p, double P, size_t l);
double upper_band_edge(double p, double P, size_t l);

double band_edge_frequency(size_t band, size_t bands, util::range<double> r);
double band_centre_frequency(size_t band, size_t bands, util::range<double> r);

////////////////////////////////////////////////////////////////////////////////

template <size_t... Ix>
auto band_edges(util::range<double> r, std::index_sequence<Ix...>) {
    return std::array<double, sizeof...(Ix)>{
            {band_edge_frequency(Ix, sizeof...(Ix) - 1, r)...}};
}

template <size_t bands>
auto band_edges(util::range<double> r) {
    return band_edges(r, std::make_index_sequence<bands + 1>{});
}

////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

template <size_t... Ix>
auto band_centres(util::range<double> r, std::index_sequence<Ix...>) {
    return std::array<double, sizeof...(Ix)>{
            {band_centre_frequency(Ix, sizeof...(Ix), r)...}};
}

template <size_t bands>
auto band_centres(util::range<double> r) {
    return band_centres(r, std::make_index_sequence<bands>{});
}

////////////////////////////////////////////////////////////////////////////////

/// frequency: normalized frequency, from 0 to 0.5
/// lower: the normalized lower band edge frequency of this band
/// lower_edge_width: half the absolute width of the lower crossover
/// upper: the normalized upper band edge frequency of this band
/// upper_edge_width: half the absolute width of the upper crossover
/// l: the slope (0 is shallow, higher is steeper)
double compute_bandpass_magnitude(double frequency,
                                  util::range<double> r,
                                  double width_factor,
                                  size_t l = 0);

double compute_lopass_magnitude(double frequency,
                                double edge,
                                double width_factor,
                                size_t l = 0);

double compute_hipass_magnitude(double frequency,
                                double edge,
                                double width_factor,
                                size_t l = 0);

}  // namespace frequency_domain
