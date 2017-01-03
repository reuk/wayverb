#pragma once

#include "hrtf/multiband.h"

#include "utilities/map_to_vector.h"
#include <iostream>

namespace wayverb {
namespace core {

template <typename It>
auto mixdown(It b, It e) {
    return util::map_to_vector(b, e, [](const auto& i) { return sum(i); });
}

template <typename It, typename Callback>
auto multiband_filter_and_mixdown(It b,
                                  It e,
                                  double sample_rate,
                                  Callback&& callback) {
    hrtf_data::multiband_filter(
            b, e, sample_rate, std::forward<Callback>(callback));
    return mixdown(b, e);
}

}  // namespace core
}  // namespace wayverb
