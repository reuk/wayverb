#pragma once

#include "utilities/aligned/vector.h"

template <typename It, typename Callback>
auto map_to_vector(It begin, It end, Callback callback) {
    using ReturnType = std::decay_t<decltype(
            std::declval<Callback>()(*std::declval<It>()))>;
    aligned::vector<ReturnType> ret{};
    ret.reserve(std::distance(begin, end));
    for (; begin != end; ++begin) {
        ret.emplace_back(callback(*begin));
    }
    return ret;
}
