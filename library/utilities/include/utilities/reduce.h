#pragma once

#include <array>
#include <functional>
#include <type_traits>

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
    return reduce(f(a, std::get<i - 1>(data)),
                  data,
                  std::integral_constant<size_t, i - 1>{},
                  f);
}

template <typename Func, typename T>
constexpr auto reduce(const T& data, const Func& f = Func()) {
    return reduce(data.back(),
                  data,
                  std::integral_constant<size_t, std::tuple_size<T>{} - 1>{},
                  f);
}

template <typename Func, typename T, typename Start>
constexpr auto reduce(const T& data,
                      const Start& starting_value,
                      const Func& f = Func()) {
    return reduce(starting_value,
                  data,
                  std::integral_constant<size_t, std::tuple_size<T>{}>{},
                  f);
}

struct identity final {
    template <typename T>
    constexpr auto operator()(T&& t) const {
        return t;
    }
};

template <typename A, typename Func = identity>
constexpr bool any(const A& arr, const Func& func = Func()) {
    return reduce(arr, false, std::logical_or<>());
}

template <typename A, typename Func = identity>
constexpr bool all(const A& arr, const Func& func = Func()) {
    return reduce(arr, true, std::logical_and<>());
}
