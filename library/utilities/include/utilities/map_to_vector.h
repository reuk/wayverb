#pragma once

#include "utilities/aligned/vector.h"

#include <algorithm>

namespace util {

template <typename It, typename Callback>
auto map_to_vector(It b, It e, const Callback &callback) {
    using ReturnType = std::decay_t<decltype(
            std::declval<Callback>()(*std::declval<It>()))>;
    aligned::vector<ReturnType> ret;
    ret.reserve(std::distance(b, e));
    std::transform(b, e, std::back_inserter(ret), callback);
    return ret;
}

}  // namespace util
