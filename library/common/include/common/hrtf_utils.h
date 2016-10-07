#pragma once

#include "common/cl/iterator.h"
#include "common/cl/scene_structs.h"
#include "common/cl/traits.h"
#include "common/hrtf.h"

#include "utilities/aligned/vector.h"
#include "utilities/map_to_vector.h"
#include "utilities/range.h"

#include "frequency_domain/envelope.h"
#include "frequency_domain/filter.h"

enum class hrtf_channel { left, right };

template <typename It>
aligned::vector<float> mixdown(It begin, It end) {
    return map_to_vector(begin, end, [](const auto& i) { return sum(i); });
}

template <typename It>
void multiband_filter(It begin, It end, double sample_rate) {
    frequency_domain::filter filter{
            static_cast<size_t>(std::distance(begin, end)) * 2};

    //  TODO use global band edge frequencies rather than computing
    //  them fresh here.

    constexpr auto num_bands{8};

    const range<double> audible_range{20 / sample_rate, 20000 / sample_rate};

    const auto band_edges{frequency_domain::band_edge_frequencies<num_bands>(
            audible_range.get_min(), audible_range.get_max())};
    const auto edge_widths{frequency_domain::band_edge_widths<num_bands>(
            audible_range.get_min(), audible_range.get_max(), 1)};

    for (auto i{0ul}; i != num_bands; ++i) {
        const auto b{make_cl_type_iterator(begin, i)};
        const auto e{make_cl_type_iterator(end, i)};
        filter.run(b, e, b, [&](auto cplx, auto freq) {
            return cplx * static_cast<float>(
                                  frequency_domain::compute_bandpass_magnitude(
                                          freq,
                                          band_edges[i],
                                          band_edges[i + 1],
                                          edge_widths[i + 0],
                                          edge_widths[i + 1],
                                          0));
        });
    }
}

template <typename T>
aligned::vector<float> multiband_filter_and_mixdown(T collection,
                                                    double sample_rate) {
    using std::begin;
    using std::end;
    multiband_filter(begin(collection), end(collection), sample_rate);
    return mixdown(begin(collection), end(collection));
}
