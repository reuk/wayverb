#pragma once

#include "utilities/tuple_like.h"

template <typename Callback, typename T>
constexpr auto foldl_params(Callback&&, T&& t) -> decltype(std::forward<T>(t)) {
    return std::forward<T>(t);
}

template <typename Callback, typename Acc, typename T, typename... Ts>
constexpr auto foldl_params(Callback&& callback, Acc&& acc, T&& t, Ts&&... ts) {
    return foldl_params(std::forward<Callback>(callback),
                        callback(std::forward<Acc>(acc), std::forward<T>(t)),
                        std::forward<Ts>(ts)...);
}

template <typename Callback, typename T, size_t... Ix>
constexpr auto foldl(Callback&& callback,
                     const T& t,
                     std::index_sequence<Ix...>) {
    return foldl_params(std::forward<Callback>(callback),
                        tuple_like_getter<Ix>(t)...);
}

template <typename Callback, typename T>
constexpr auto foldl(Callback&& callback, const T& t) {
    return foldl(std::forward<Callback>(callback),
                 t,
                 std::make_index_sequence<tuple_like_size_v<T>>{});
}

template <typename Callback, typename Init, typename T, size_t... Ix>
constexpr auto foldl(Callback&& callback,
                     const Init& init,
                     const T& t,
                     std::index_sequence<Ix...>) {
    return foldl_params(std::forward<Callback>(callback),
                        init,
                        tuple_like_getter<Ix>(t)...);
}

template <typename Callback, typename Init, typename T>
constexpr auto foldl(Callback&& callback, const Init& init, const T& t) {
    return foldl(std::forward<Callback>(callback),
                 init,
                 t,
                 std::make_index_sequence<tuple_like_size_v<T>>{});
}
