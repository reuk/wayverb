#pragma once

#include <vector>

enum class HrtfChannel { left, right };

/// Given a vector of frequency band signals, a vector of band edges, and
/// a sampling rate, filter the bands and sum them down.

std::vector<float> mixdown(const std::vector<std::vector<float>>& data);
std::vector<std::vector<float>> mixdown(
        const std::vector<std::vector<std::vector<float>>>& data);

std::vector<std::vector<float>> multiband_filter(
        const std::vector<std::vector<float>> bands,
        const std::vector<double>& band_edges,
        double sample_rate);

std::vector<float> multiband_filter_and_mixdown(
        const std::vector<std::vector<float>>& bands, double sample_rate);

/// Run a single-channel attenuator repeatedly over an input.
template <typename Callback, typename It>
auto run_attenuation(It begin,
                     It end,
                     const Callback& callback) {
    using ret_type = decltype(std::declval<Callback>()(*begin));
    std::vector<ret_type> ret;
    ret.reserve(std::distance(begin, end));
    std::transform(begin, end, std::back_inserter(ret), callback);
    return ret;
}
