#include "common/hrtf_utils.h"
#include "common/cl/iterator.h"
#include "common/filters_common.h"
#include "common/frequency_domain_filter.h"
#include "common/hrtf.h"
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
    for_each_band(sample_rate, [&](auto index, auto lo, auto hi) {
        filter::linkwitz_riley_bandpass(
                lo,
                hi,
                sample_rate,
                make_cl_type_iterator(bands.begin(), index),
                make_cl_type_iterator(bands.end(), index));
    });
    //  TODO consider a frequency-domain filter here instead.
}

aligned::vector<float> multiband_filter_and_mixdown(
        aligned::vector<volume_type> sig, double sample_rate) {
    multiband_filter(sig, sample_rate);
    return mixdown(sig);
}
