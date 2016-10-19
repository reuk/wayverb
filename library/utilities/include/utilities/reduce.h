#pragma once

#include "utilities/tuple_like.h"

template <typename Callback, typename T>
constexpr auto reduce_params(Callback&&, T&& t) {
    return t;
}

template <typename Callback, typename T, typename... Ts>
constexpr auto reduce_params(Callback&& callback, T&& t, Ts&&... ts) {
    return callback(std::forward<T>(t),
                    reduce_params(std::forward<Callback>(callback),
                                  std::forward<Ts>(ts)...));
}

template <typename Callback, typename T, size_t... Ix>
constexpr auto reduce(Callback&& callback,
                      const T& t,
                      std::index_sequence<Ix...>) {
    return reduce_params(std::forward<Callback>(callback),
                         tuple_like_getter<Ix>(t)...);
}

template <typename Callback, typename T>
constexpr auto reduce(Callback&& callback, const T& t) {
    return reduce(std::forward<Callback>(callback),
                  t,
                  std::make_index_sequence<tuple_like_size_v<T>>{});
}

template <typename Callback, typename Init, typename T, size_t... Ix>
constexpr auto reduce(Callback&& callback,
                      const Init& init,
                      const T& t,
                      std::index_sequence<Ix...>) {
    return reduce_params(std::forward<Callback>(callback),
                         init,
                         tuple_like_getter<Ix>(t)...);
}

template <typename Callback, typename Init, typename T>
constexpr auto reduce(Callback& callback, const Init& init, const T& t) {
    return reduce(std::forward<Callback>(callback),
                  init,
                  t,
                  std::make_index_sequence<tuple_like_size_v<T>>{});
}
