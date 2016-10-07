#pragma once

#include "frequency_domain/envelope.h"
#include "frequency_domain/filter.h"

#include "utilities/aligned/vector.h"
#include "utilities/map.h"
#include "utilities/range.h"

#include <array>
#include <numeric>

template <typename It>
auto band_energy(It begin,
                 It end,
                 range<double> band_range,
                 double lower_width,
                 double upper_width) {
    aligned::vector<float> band(begin, end);
    frequency_domain::filter filter{band.size() * 2};
    filter.run(
            band.begin(), band.end(), band.begin(), [&](auto cplx, auto freq) {
                return cplx *
                       static_cast<float>(
                               frequency_domain::compute_bandpass_magnitude(
                                       freq,
                                       band_range.get_min(),
                                       band_range.get_max(),
                                       lower_width,
                                       upper_width,
                                       0));
            });

    const auto band_energy{std::sqrt(
            std::accumulate(band.begin(), band.end(), 0.0, [](auto a, auto b) {
                return a + b * b;
            }))};

    return band_energy / dimensions(band_range);
}

template <size_t bands, typename It>
std::array<float, bands> per_band_energy(It begin, It end, range<double> r) {
    std::array<float, bands> ret{};

    const auto edges{frequency_domain::band_edge_frequencies<bands>(
            r.get_min(), r.get_max())};
    const auto widths{frequency_domain::band_edge_widths<bands>(
            r.get_min(), r.get_max(), 1)};

    for (auto i{0ul}; i != bands; ++i) {
        ret[i] = band_energy(begin,
                             end,
                             range<double>{edges[i], edges[i + 1]},
                             widths[i],
                             widths[i + 1]);
    }

    return ret;
}

