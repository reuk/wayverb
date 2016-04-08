#pragma once

#include "config.h"

#include <algorithm>
#include <array>
#include <cmath>
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
void run(FilterType ft,
         std::vector<std::vector<std::vector<float>>>& data,
         float sr,
         float lo_cutoff);
}

/// JsonGetter for FilterType is just a JsonEnumGetter with a specific map
template <>
struct JsonGetter<filter::FilterType>
    : public JsonEnumGetter<filter::FilterType> {
    JsonGetter(filter::FilterType& t)
            : JsonEnumGetter(
                  t,
                  {{"sinc", filter::FilterType::windowed_sinc},
                   {"onepass", filter::FilterType::biquad_onepass},
                   {"twopass", filter::FilterType::biquad_twopass},
                   {"linkwitz_riley", filter::FilterType::linkwitz_riley}}) {
    }
};
