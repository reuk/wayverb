#include "common/cl/iterator.h"
#include "common/filters_common.h"
#include "common/frequency_domain_filter.h"
#include "common/hrtf.h"
#include "common/hrtf_utils.h"
#include "common/map_to_vector.h"

/// Sum a collection of vectors of the same length into a single vector
aligned::vector<float> mixdown(const aligned::vector<volume_type>& data) {
    return map_to_vector(data, [](const auto& i) { return sum(i); });
}

aligned::vector<aligned::vector<float>> mixdown(
        const aligned::vector<aligned::vector<volume_type>>& data) {
    return map_to_vector(data, [](const auto& i) { return mixdown(i); });
}

void multiband_filter(aligned::vector<volume_type>& bands, double sample_rate) {
    constexpr auto num_bands{8};
    fast_filter filter{bands.size()};

    const auto lower{20 / sample_rate}, upper{20000 / sample_rate};

    const auto band_edges{band_edge_frequencies<num_bands>(lower, upper)};

    constexpr auto overlap{1};
    const auto edge_widths{band_edge_widths<num_bands>(lower, upper, overlap)};

    constexpr auto l{0};

    for (auto i{0ul}; i != num_bands; ++i) {
        const auto begin{make_cl_type_iterator(bands.begin(), i)};
        const auto end{make_cl_type_iterator(bands.end(), i)};
        filter.filter(begin, end, begin, [&](auto cplx, auto freq) {
            return cplx * compute_bandpass_magnitude(freq,
                                                     band_edges[i + 0],
                                                     edge_widths[i + 0],
                                                     band_edges[i + 1],
                                                     edge_widths[i + 1],
                                                     l);
        });
    }
}

aligned::vector<float> multiband_filter_and_mixdown(
        aligned::vector<volume_type> sig, double sample_rate) {
    multiband_filter(sig, sample_rate);
    return mixdown(sig);
}
