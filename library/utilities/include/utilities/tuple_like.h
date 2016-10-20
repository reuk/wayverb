#pragma once

#include <array>
#include <functional>
#include <tuple>
#include <type_traits>

template <typename T>
using decay_const_ref_t = std::remove_cv_t<std::remove_reference_t<T>>;

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
    using type = std::integral_constant<size_t, I>;
};

template <typename T>
using tuple_like_size_t = typename tuple_like_size<T>::type;

template <typename T>
constexpr auto tuple_like_size_v = tuple_like_size_t<T>{};

//----------------------------------------------------------------------------//

template <size_t I, typename T>
constexpr auto tuple_like_getter(T&& t) -> decltype(std::get<I>(t)) {
    return std::get<I>(t);
}

template <size_t I, typename T, size_t Ix>
constexpr auto& tuple_like_getter(T (&x)[Ix]) {
    static_assert(I < Ix, "index out of range");
    return x[I];
}

template <size_t I, typename T, size_t Ix>
constexpr const auto& tuple_like_getter(const T (&x)[Ix]) {
    static_assert(I < Ix, "index out of range");
    return x[I];
}

