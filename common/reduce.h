#pragma once

#include <type_traits>
#include <array>

template <size_t I>
struct Indexer : std::integral_constant<decltype(I), I> {
    using Next = Indexer<I - 1>;
};

template <typename Func, typename A, typename T>
auto reduce(const A& a, const T&, Indexer<0>, const Func& = Func{}) {
    return a;
}

template <typename Func, typename A, typename T, typename Ind>
auto reduce(const A& a, const T& data, Ind i = Ind{}, const Func& f = Func{}) {
    return reduce(f(a, std::get<i - 1>(data)), data, typename Ind::Next{}, f);
}

template <typename Func, typename T>
auto reduce(const T& data, const Func& f = Func()) {
    return reduce(data.back(), data, Indexer<std::tuple_size<T>() - 1>{}, f);
}
