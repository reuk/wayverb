#pragma once

#include "hrtf/multiband.h"

#include "utilities/map_to_vector.h"

template <typename It>
auto mixdown(It begin, It end) {
    return map_to_vector(begin, end, [](const auto& i) { return sum(i); });
}

template <typename It, typename Callback>
aligned::vector<float> multiband_filter_and_mixdown(It begin,
                                                    It end,
                                                    double sample_rate,
                                                    const Callback& callback) {
    hrtf_data::multiband_filter(begin, end, sample_rate, callback);
    return mixdown(begin, end);
}
