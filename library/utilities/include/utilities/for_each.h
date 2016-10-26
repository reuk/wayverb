#pragma once

#include "utilities/tuple_like.h"

#include <initializer_list>
#include <utility>

template <typename Func, typename... Ts>
void for_each_params(Func&& func, Ts&&... ts) {
    (void)std::initializer_list<int>{
            ((void)std::forward<Func>(func)(std::forward<Ts>(ts)), 0)...};
}

template <typename Func, typename T, size_t... Ix>
void for_each(Func&& func, T&& t, std::index_sequence<Ix...>) {
    for_each_params(std::forward<Func>(func),
                    tuple_like_getter<Ix>(std::forward<T>(t))...);
}

template <typename Func, typename T>
void for_each(Func&& func, T&& t) {
    for_each(std::forward<Func>(func),
             std::forward<T>(t),
             std::make_index_sequence<
                     tuple_like_size_v<decay_const_ref_t<T>>>{});
}
