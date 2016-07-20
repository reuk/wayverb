#pragma once

#include "common/aligned/vector.h"

enum class HrtfChannel { left, right };

/// Given a vector of frequency band signals, a vector of band edges, and
/// a sampling rate, filter the bands and sum them down.

aligned::vector<float> mixdown(
        const aligned::vector<aligned::vector<float>>& data);
aligned::vector<aligned::vector<float>> mixdown(
        const aligned::vector<aligned::vector<aligned::vector<float>>>& data);

aligned::vector<aligned::vector<float>> multiband_filter(
        const aligned::vector<aligned::vector<float>> bands,
        const aligned::vector<double>& band_edges,
        double sample_rate);

aligned::vector<float> multiband_filter_and_mixdown(
        const aligned::vector<aligned::vector<float>>& bands,
        double sample_rate);

/// Run a single-channel attenuator repeatedly over an input.
template <typename Callback, typename It>
auto run_attenuation(It begin, It end, const Callback& callback) {
    using ret_type = decltype(std::declval<Callback>()(*begin));
    aligned::vector<ret_type> ret;
    ret.reserve(std::distance(begin, end));
    std::transform(begin, end, std::back_inserter(ret), callback);
    return ret;
}
