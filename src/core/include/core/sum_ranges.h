#pragma once

#include "utilities/foldl.h"
#include "utilities/for_each.h"

namespace core {

template <typename... Ts>
auto sum_params(Ts&&... ts) {
    return util::foldl_params(std::plus<>{}, std::forward<Ts>(ts)...);
}

template <typename It, typename... Its>
auto sum_ranges(It b, It e, Its... its) {
    using value_type = decltype(sum_params(*b, *its...));
    util::aligned::vector<value_type> ret;
    ret.reserve(std::distance(b, e));
    while (b != e) {
        ret.emplace_back(sum_params(*b++, (*its++)...));
    }
    return ret;
}

template <typename T, typename... Ts>
auto sum_vectors(T t, Ts... ts) {
    const auto max_len = util::foldl_params(
            [](auto a, auto b) { return std::max(a.size(), b.size()); },
            t,
            ts...);
    util::for_each_params([&](auto& vec) { vec.resize(max_len); }, t, ts...);
    return sum_ranges(begin(t), end(t), begin(ts)...);
}

}  // namespace core
