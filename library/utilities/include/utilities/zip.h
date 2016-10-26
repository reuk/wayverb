#pragma once

#include "utilities/tuple_like.h"

#include <tuple>

template <size_t Ix, typename... Ts>
auto zip(Ts&&... ts) {
    return std::make_tuple(std::get<Ix>(std::forward<Ts>(ts))...);
}

template <size_t... Ix, typename... Ts>
auto zip(std::index_sequence<Ix...>, Ts&&... ts) {
    return std::make_tuple(zip<Ix>(std::forward<Ts>(ts)...)...);
}

template <typename T, typename... Ts>
auto zip(T&& t, Ts&&... ts) {
    return zip(
            std::make_index_sequence<tuple_like_size_v<decay_const_ref_t<T>>>{},
            std::forward<T>(t),
            std::forward<Ts>(ts)...);
}

