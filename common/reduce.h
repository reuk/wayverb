#pragma once

#include <type_traits>
#include <array>

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
    auto operator()(const T& t) const {
        return t;
    }
};

template <typename A, typename Func = Identity>
constexpr bool any(const A& arr, const Func& func = Func()) {
    return reduce(arr, false, [&func](auto i, auto j) { return i || func(j); });
}

template <typename A, typename Func = Identity>
constexpr bool all(const A& arr, const Func& func = Func()) {
    return reduce(arr, true, [&func](auto i, auto j) { return i && func(j); });
}

constexpr int popcount(unsigned long long t) {
    int ret = 0;
    for (; t; t &= t - 1)
        ++ret;
    return ret;
}

template <typename Flag>
struct FlagSet {
    FlagSet(Flag flag)
            : flag(flag) {
    }
    template <typename T>
    auto operator()(T t) const {
        return t & flag;
    }

private:
    Flag flag;
};

template <typename T, typename A>
constexpr bool any_flags_set(T t, const A& arr) {
    return any(arr, FlagSet<T>(t));
}

template <typename T, typename A>
constexpr bool all_flags_set(T t, const A& arr) {
    return all(arr, FlagSet<T>(t));
}
