#pragma once

#include <vector>

namespace filter {

/// Enum denoting available filter types.
enum class FilterType {
    windowed_sinc,
    biquad_onepass,
    biquad_twopass,
    linkwitz_riley,
};

/// Given a filter type and a vector of vector of float, return the
/// parallel-filtered and summed data, using the specified filtering method.
std::vector<std::vector<std::vector<float>>> run(
        FilterType ft,
        const std::vector<std::vector<std::vector<float>>>& data,
        float sr,
        float lo_cutoff);
}  // namespace filter
