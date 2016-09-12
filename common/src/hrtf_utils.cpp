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

float band_edge_frequency(int band, size_t bands, float lower, float upper) {
    return lower * std::pow(upper / lower, band / static_cast<float>(bands));
}

template <size_t bands>
std::array<float, bands + 1> band_edge_frequencies(float lower, float upper) {
    std::array<float, bands + 1> ret;
    for (auto i{0u}; i != bands + 1; ++i) {
        ret[i] = band_edge_frequency(i, bands, lower, upper);
    }
    return ret;
}

template <size_t bands>
std::array<float, bands + 1> band_centre_frequencies(float lower, float upper) {
    std::array<float, bands + 1> ret;
    for (auto i{0ul}; i != ret.size(); ++i) {
        ret[i] = band_edge_frequency(i * 2 + 1, bands * 2, lower, upper);
    }
    return ret;
}

template <size_t bands>
std::array<float, bands + 1> band_edge_widths(float lower,
                                              float upper,
                                              float overlap) {
    std::array<float, bands + 1> ret;
    for (int i{0}; i != ret.size(); ++i) {
        ret[i] = std::abs(
                (band_edge_frequency(i * 2, bands * 2, lower, upper) -
                 band_edge_frequency(i * 2 + 1, bands * 2, lower, upper)) *
                overlap);
    }
    return ret;
}

/// frequency: normalized frequency, from 0 to 0.5
/// lower: the normalized lower band edge frequency of this band
/// upper: the normalized upper band edge frequency of this band
float compute_magnitude(float frequency,
                        float lower,
                        float lower_edge_width,
                        float upper,
                        float upper_edge_width,
                        size_t l) {
    if (frequency < lower - lower_edge_width ||
        upper + upper_edge_width <= frequency) {
        return 0;
    }

    const auto lower_p{frequency - lower};
    if (-lower_edge_width <= lower_p && lower_p < lower_edge_width) {
        return lower_band_edge(lower, lower_p, lower_edge_width, l);
    }

    const auto upper_p{frequency - upper};
    if (-upper_edge_width <= upper_p && upper_p < upper_edge_width) {
        return upper_band_edge(upper, upper_p, upper_edge_width, l);
    }

    return 1;
}

void multiband_filter(aligned::vector<volume_type>& bands, double sample_rate) {
#if 0
    for_each_band(sample_rate, [&](auto index, auto lo, auto hi) {
        filter::linkwitz_riley_bandpass(
                lo,
                hi,
                sample_rate,
                make_cl_type_iterator(bands.begin(), index),
                make_cl_type_iterator(bands.end(), index));
    });
#else
    constexpr auto num_bands{8};
    fast_filter filter{bands.size()};

    const auto lower{20 / sample_rate}, upper{20000 / sample_rate};

    const auto band_edges{band_edge_frequencies<num_bands>(lower, upper)};

    constexpr auto overlap{0.5};
    const auto edge_widths{band_edge_widths<num_bands>(lower, upper, overlap)};

    constexpr auto l{0};

    for (auto i{0ul}; i != num_bands; ++i) {
        const auto begin{make_cl_type_iterator(bands.begin(), i)};
        const auto end{make_cl_type_iterator(bands.end(), i)};
        filter.filter(begin, end, begin, [&](auto cplx, auto freq) {
            return cplx * compute_magnitude(freq,
                                            band_edges[i + 0],
                                            edge_widths[i + 0],
                                            band_edges[i + 1],
                                            edge_widths[i + 1],
                                            l);
        });
    }

#endif
}

aligned::vector<float> multiband_filter_and_mixdown(
        aligned::vector<volume_type> sig, double sample_rate) {
    multiband_filter(sig, sample_rate);
    return mixdown(sig);
}
