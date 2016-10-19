#pragma once

#include "utilities/is_tuple.h"
#include "utilities/tuple_like.h"

//  arrays

template <typename Func, typename T, size_t... Ix>
constexpr auto map(Func&& func,
                   T&& t,
                   std::index_sequence<Ix...>,
                   std::enable_if_t<!is_tuple_v<std::decay_t<T>>, int> = 0) {
    using value_type = decltype(func(tuple_like_getter<0>(t)));
    return std::array<value_type, sizeof...(Ix)>{
            {func(tuple_like_getter<Ix>(t))...}};
}

template <typename Func, typename T, size_t... Ix>
constexpr auto map(Func&& func,
                   T&& t,
                   std::index_sequence<Ix...>,
                   std::enable_if_t<is_tuple_v<std::decay_t<T>>, int> = 0) {
    return std::make_tuple(func(tuple_like_getter<Ix>(t))...);
}

//  master dispatching function ----------------------------------------------//

template <typename Func, typename T>
constexpr auto map(Func&& func, T&& t) {
    return map(std::forward<Func>(func),
               std::forward<T>(t),
               std::make_index_sequence<
                       tuple_like_size_v<decay_const_ref_t<T>>>{});
}
