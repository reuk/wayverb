#pragma once

#include "config.h"

#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

/// Enum denoting available filter types.
enum FilterType {
    FILTER_TYPE_WINDOWED_SINC,
    FILTER_TYPE_BIQUAD_ONEPASS,
    FILTER_TYPE_BIQUAD_TWOPASS,
    FILTER_TYPE_LINKWITZ_RILEY
};

/// Given a filter type and a vector of vector of float, return the
/// parallel-filtered and summed data, using the specified filtering method.
void filter(FilterType ft,
            std::vector<std::vector<std::vector<float>>>& data,
            float sr,
            float lo_cutoff);

/// JsonGetter for FilterType is just a JsonEnumGetter with a specific map
template <>
struct JsonGetter<FilterType> : public JsonEnumGetter<FilterType> {
    JsonGetter(FilterType& t)
            : JsonEnumGetter(t,
                             {{"sinc", FILTER_TYPE_WINDOWED_SINC},
                              {"onepass", FILTER_TYPE_BIQUAD_ONEPASS},
                              {"twopass", FILTER_TYPE_BIQUAD_TWOPASS},
                              {"linkwitz_riley", FILTER_TYPE_LINKWITZ_RILEY}}) {
    }
};
