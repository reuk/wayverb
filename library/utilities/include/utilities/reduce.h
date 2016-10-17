#pragma once

#include <array>
#include <functional>
#include <type_traits>

//----------------------------------------------------------------------------//

template <typename T>
struct tuple_like_size;

template <typename T, size_t I>
struct tuple_like_size<std::array<T, I>> final {
    using type = std::tuple_size<std::array<T, I>>;
};

template <typename... Ts>
struct tuple_like_size<std::tuple<Ts...>> final {
    using type = std::tuple_size<std::tuple<Ts...>>;
};

template <typename T, size_t I>
struct tuple_like_size<T[I]> final {
    using type = std::extent<T[I]>;
};

template <typename T>
using tuple_like_size_t = typename tuple_like_size<T>::type;

template <typename T>
constexpr auto tuple_like_size_v{tuple_like_size_t<T>{}};

//----------------------------------------------------------------------------//

template <size_t I, typename T, size_t Ix>
constexpr auto& tuple_like_getter(std::array<T, Ix>& x) {
    return std::get<I>(x);
}

template <size_t I, typename T, size_t Ix>
constexpr const auto& tuple_like_getter(const std::array<T, Ix>& x) {
    return std::get<I>(x);
}

template <size_t I, typename... Ts>
constexpr auto& tuple_like_getter(std::tuple<Ts...>& x) {
    return std::get<I>(x);
}

template <size_t I, typename... Ts>
constexpr const auto& tuple_like_getter(const std::tuple<Ts...>& x) {
    return std::get<I>(x);
}

template <size_t I, typename T, size_t Ix>
constexpr auto& tuple_like_getter(T (&x)[Ix]) {
    return x[I];
}

template <size_t I, typename T, size_t Ix>
constexpr const auto& tuple_like_getter(const T (&x)[Ix]) {
    return x[I];
}

//----------------------------------------------------------------------------//

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
