#pragma once

#include "utilities/sequential_foreach.h"
#include "utilities/tuple_like.h"

template <typename Func, typename T, size_t... Ix>
void for_each(Func&& func, T&& t, std::index_sequence<Ix...>) {
    sequential_foreach(std::forward<Func>(func),
                       tuple_like_getter<Ix>(std::forward<T>(t))...);
}

template <typename Func, typename T>
void for_each(Func&& func, T&& t) {
    for_each(std::forward<Func>(func),
             std::forward<T>(t),
             std::make_index_sequence<
                     tuple_like_size_v<decay_const_ref_t<T>>>{});
}
