#pragma once

#include <array>
#include <functional>
#include <type_traits>

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

template <typename Func, typename A, typename T>
constexpr auto reduce(const A& a,
                      const T&,
                      std::integral_constant<size_t, 0>,
                      const Func& = Func{}) {
    return a;
}

template <typename Func, typename A, typename T, size_t i>
constexpr auto reduce(const A& a,
                      const T& data,
                      std::integral_constant<size_t, i>,
                      const Func& f = Func{}) {
    return reduce(f(a, tuple_like_getter<i - 1>(data)),
                  data,
                  std::integral_constant<size_t, i - 1>{},
                  f);
}

template <typename Func, typename T>
constexpr auto reduce(const T& data, const Func& f = Func()) {
    return reduce(data.back(),
                  data,
                  std::integral_constant<size_t, tuple_like_size_v<T> - 1>{},
                  f);
}

template <typename Func, typename T, typename Start>
constexpr auto reduce(const T& data,
                      const Start& starting_value,
                      const Func& f = Func()) {
    return reduce(starting_value,
                  data,
                  std::integral_constant<size_t, tuple_like_size_v<T>>{},
                  f);
}
