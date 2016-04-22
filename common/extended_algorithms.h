#pragma once

#include "stl_wrappers.h"

#include <vector>

namespace proc {

//  TODO(reuben): map, reduce, filter

template <typename... Ts>
struct min_tuple_size;

template <typename T>
struct min_tuple_size<T> {
    constexpr static auto value =
        std::tuple_size<typename std::decay<T>::type>::value;
};

template <typename T, typename... Ts>
struct min_tuple_size<T, Ts...> {
    constexpr static auto value =
        std::min(min_tuple_size<T>::value, min_tuple_size<Ts...>::value);
};

template <typename... Ts>
constexpr auto get_min_tuple_size(Ts&&... ts) {
    return min_tuple_size<Ts...>::value;
}

template <size_t i, typename... Ts>
constexpr auto zip_impl(Ts&&... ts) {
    return std::make_tuple(std::get<i>(ts)...);
}

template <size_t... Ix, typename... Ts>
constexpr auto zip_to_tuple(std::index_sequence<Ix...>, Ts&&... ts) {
    return std::make_tuple(zip_impl<Ix>(std::forward<Ts>(ts)...)...);
}

template <typename... Ts>
constexpr auto zip_to_tuple(Ts&&... ts) {
    return zip_to_tuple(
        std::make_index_sequence<min_tuple_size<Ts...>::value>(), ts...);
}

template <size_t... Ix, typename T, typename... Ts>
constexpr auto tuple_to_array(std::index_sequence<Ix...>,
                              const std::tuple<T, Ts...>& ts) {
    return std::array<T, std::index_sequence<Ix...>::size()>{
        {std::get<Ix>(ts)...}};
}

template <typename... Ts>
constexpr auto tuple_to_array(const std::tuple<Ts...>& t) {
    return tuple_to_array(
        std::make_index_sequence<std::tuple_size<std::tuple<Ts...>>::value>(),
        t);
}

template <typename... Ts>
constexpr auto zip_to_array(Ts&&... ts) {
    return tuple_to_array(zip_to_tuple(std::forward<Ts>(ts)...));
}

template <typename T>
auto min_size(const T& t) {
    return t.size();
}

template <typename T, typename... Ts>
auto min_size(const T& t, Ts&&... ts) {
    return std::min(min_size(t), min_size(std::forward<Ts>(ts)...));
}

template <typename T>
struct zipped_type_impl;

template <typename T>
struct zipped_type_impl<std::vector<T>> {
    using apply = T;
};

template <typename T, size_t I>
struct zipped_type_impl<std::array<T, I>> {
    using apply = T;
};

template <typename T, typename... Ts>
struct zipped_type_impl<std::tuple<T, Ts...>> {
    using apply = T;
};

template <typename... Ts>
struct zipped_type {
    using apply = std::tuple<
        typename zipped_type_impl<typename std::decay<Ts>::type>::apply...>;
};

template <typename... Ts>
auto zip(Ts&&... ts) {
    using zipped = typename zipped_type<Ts...>::apply;
    std::vector<zipped> ret(min_size(ts...));
    for (auto i = 0; i != ret.size(); ++i) {
        ret[i] = std::make_tuple(ts[i]...);
    }
    return ret;
}

template <typename T>
constexpr bool c_eq(std::index_sequence<>, const T& a) {
    return true;
}

template <size_t X, size_t... Ix, typename T>
constexpr bool c_eq(std::index_sequence<X, Ix...>, const T& a) {
    auto item = std::get<X>(a);
    return std::get<0>(item) == std::get<1>(item) &&
           c_eq(std::index_sequence<Ix...>(), a);
}

template <typename T>
constexpr bool c_eq(const T& a) {
    return c_eq(std::make_index_sequence<std::tuple_size<T>::value>(), a);
}

template <typename T, size_t I>
constexpr bool c_eq(const std::array<T, I>& a, const std::array<T, I>& b) {
    return c_eq(zip_to_array(a, b));
}

template <typename Fun, typename Tup, size_t... Ix>
constexpr static auto invoke(Fun&& fun,
                             const Tup& tup,
                             std::index_sequence<Ix...>) {
    return std::forward<Fun>(fun)(std::get<Ix>(tup)...);
}
template <typename Fun, typename Tup>
constexpr static auto invoke(Fun&& fun, Tup&& tup) {
    return invoke(
        std::forward<Fun>(fun),
        std::forward<Tup>(tup),
        std::make_index_sequence<
            std::tuple_size<typename std::decay<Tup>::type>::value>());
}

//  this might not behave properly regarding forwarding
template <typename Fun>
struct InvokeFunctor {
    constexpr explicit InvokeFunctor(const Fun& fun)
            : fun(fun) {
    }
    template <typename Tup>
    constexpr auto operator()(Tup&& tup) const {
        return invoke(fun, tup);
    }
    Fun fun;
};

template <typename T, typename Callback, size_t... Ix>
constexpr auto map(std::index_sequence<Ix...>,
                   const T& t,
                   const Callback& callback) {
    return std::array<decltype(callback(std::get<0>(t))),
                      std::tuple_size<T>::value>{
        {callback(std::get<Ix>(t))...}};
}

template <typename T, typename Callback>
constexpr auto map(const T& t, const Callback& callback) {
    return map(
        std::make_index_sequence<std::tuple_size<T>::value>(), t, callback);
}
}  // namespace proc
