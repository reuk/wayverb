#pragma once

#include <tuple>

namespace util {

template <typename T>
struct is_tuple final {
    using type = std::false_type;
};

template <typename... Ts>
struct is_tuple<std::tuple<Ts...>> final {
    using type = std::true_type;
};

template <typename T>
using is_tuple_t = typename is_tuple<T>::type;

template <typename T>
constexpr auto is_tuple_v = is_tuple_t<T>{};

}  // namespace util
