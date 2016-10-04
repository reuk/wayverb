#pragma once

#include <array>
#include <tuple>

template <typename Func, typename T, size_t num, size_t... ind>
constexpr auto map(const std::array<T, num>& t,
                   std::index_sequence<ind...>,
                   const Func& func) {
    using value_type = decltype(func(t[0]));
    return std::array<value_type, num>{{func(t[ind])...}};
}

template <typename Func, typename T, size_t num>
constexpr auto map(const std::array<T, num>& t, const Func& func) {
    return map(t, std::make_index_sequence<num>{}, func);
}

template <typename Func, typename... Ts, size_t... ind>
constexpr auto map(const std::tuple<Ts...>& tup,
                   std::index_sequence<ind...>,
                   const Func& func) {
    return std::make_tuple(func(std::get<ind>(tup))...);
}

template <typename Func, typename... Ts>
constexpr auto map(const std::tuple<Ts...>& t, const Func& func) {
    return map(t, std::make_index_sequence<sizeof...(Ts)>{}, func);
}

