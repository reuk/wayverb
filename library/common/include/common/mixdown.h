#pragma once

#include "frequency_domain/multiband_filter.h"

#include "utilities/map_to_vector.h"

template <typename It>
auto mixdown(It begin, It end) {
    return map_to_vector(begin, end, [](const auto& i) { return sum(i); });
}

template <size_t bands, typename It, typename Callback>
aligned::vector<float> multiband_filter_and_mixdown(It begin,
                                                    It end,
                                                    range<double> nrange,
                                                    const Callback& callback) {
    frequency_domain::multiband_filter<bands>(begin, end, nrange, callback);
    return mixdown(begin, end);
}
