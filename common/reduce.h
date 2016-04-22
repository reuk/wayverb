#pragma once

#include <array>
#include <functional>
#include <type_traits>

template <size_t I>
struct Indexer : std::integral_constant<decltype(I), I> {
    using Next = Indexer<I - 1>;
};

template <typename Func, typename A, typename T>
constexpr auto reduce(const A& a, const T&, Indexer<0>, const Func& = Func{}) {
    return a;
}

template <typename Func, typename A, typename T, typename Ind>
constexpr auto reduce(const A& a,
                      const T& data,
                      Ind i = Ind{},
                      const Func& f = Func{}) {
    return reduce(f(a, std::get<i - 1>(data)), data, typename Ind::Next{}, f);
}

template <typename Func, typename T>
constexpr auto reduce(const T& data, const Func& f = Func()) {
    return reduce(data.back(), data, Indexer<std::tuple_size<T>{} - 1>{}, f);
}

template <typename Func, typename T, typename Start>
constexpr auto reduce(const T& data,
                      const Start& starting_value,
                      const Func& f = Func()) {
    return reduce(starting_value, data, Indexer<std::tuple_size<T>{}>{}, f);
}

struct Identity {
    template <typename T>
    constexpr auto operator()(const T& t) const {
        return t;
    }
};

template <typename A, typename Func = Identity>
constexpr bool any(const A& arr, const Func& func = Func()) {
    return reduce(arr, false, std::logical_or<>());
}

template <typename A, typename Func = Identity>
constexpr bool all(const A& arr, const Func& func = Func()) {
    return reduce(arr, true, std::logical_and<>());
}
