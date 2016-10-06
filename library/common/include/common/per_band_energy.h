#pragma once

#include "common/aligned/vector.h"
#include "common/frequency_domain_filter.h"
#include "common/map.h"
#include "common/range.h"

#include <array>
#include <numeric>

template <typename It>
auto band_energy(It begin,
                 It end,
                 util::range<double> band_range,
                 double lower_width,
                 double upper_width) {
    aligned::vector<float> band(begin, end);
    fast_filter filter{band.size() * 2};
    filter.filter(
            band.begin(), band.end(), band.begin(), [&](auto cplx, auto freq) {
                return cplx *
                       static_cast<float>(compute_bandpass_magnitude(
                               freq, band_range, lower_width, upper_width, 0));
            });

    const auto band_energy{std::sqrt(
            std::accumulate(band.begin(), band.end(), 0.0, [](auto a, auto b) {
                return a + b * b;
            }))};

    return band_energy / dimensions(band_range);
} 

template <size_t bands, typename It>
std::array<float, bands> per_band_energy(It begin,
                                         It end,
                                         util::range<double> range) {
    std::array<float, bands> ret{};

    const auto edges{band_edge_frequencies<bands>(range)};
    const auto widths{band_edge_widths<bands>(range, 1)};

    for (auto i{0ul}; i != bands; ++i) {
        ret[i] = band_energy(begin,
                             end,
                             util::range<double>{edges[i], edges[i + 1]},
                             widths[i],
                             widths[i + 1]);
    }

    return ret;
}

