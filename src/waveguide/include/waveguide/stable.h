#pragma once

#include "utilities/tuple_like.h"

#include <cmath>

namespace waveguide {
namespace detail {

template <typename T, size_t... Ix>
constexpr auto make_next_array(T&& a, double rci, std::index_sequence<Ix...>) {
    constexpr auto size = sizeof...(Ix);
    return std::array<double, size>{
            {(util::tuple_like_getter<Ix>(a) -
              util::tuple_like_getter<size - Ix>(a) * rci) /
             (1 - rci * rci)...}};
}

template <typename T>
constexpr auto make_next_array(T&& a, double rci) {
    constexpr auto size = util::tuple_like_size_v<util::decay_const_ref_t<T>>;
    return make_next_array(
            std::forward<T>(a), rci, std::make_index_sequence<size - 1>{});
}

}  // namespace detail

template <typename T,
          std::enable_if_t<
                  util::tuple_like_size_v<util::decay_const_ref_t<T>> == 1,
                  int> = 0>
constexpr auto is_stable(T&&) {
    return true;
}

/// Should be called on autoregressive coefficients in ascending powers,
/// i.e. the denominator of a standard filter transfer function.
template <typename T,
          std::enable_if_t<
                  util::tuple_like_size_v<util::decay_const_ref_t<T>> != 1,
                  int> = 0>
constexpr auto is_stable(T&& a) {
    constexpr auto size = util::tuple_like_size_v<util::decay_const_ref_t<T>>;
    const auto rci = util::tuple_like_getter<size - 1>(a);
    if (1 <= std::abs(rci)) {
        return false;
    }
    return is_stable(detail::make_next_array(std::forward<T>(a), rci));
}

}  // namespace waveguide
