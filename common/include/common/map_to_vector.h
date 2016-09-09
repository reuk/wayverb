#pragma once

#include "common/aligned/vector.h"

template <typename It, typename Callback>
auto map_to_vector(It begin, It end, Callback callback) {
    using ret_type = decltype(std::declval<Callback>()(*begin));
    aligned::vector<ret_type> ret;
    ret.reserve(std::distance(begin, end));
    for (; begin != end; ++begin) {
        ret.emplace_back(callback(*begin));
    }
    return ret;
}

template <typename T, typename Callback>
auto map_to_vector(const T& t, Callback callback) {
    using std::begin;
    using std::end;
    return map_to_vector(begin(t), end(t), callback);
}
