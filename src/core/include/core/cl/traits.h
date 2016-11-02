#pragma once

#include "core/cl/include.h"

#include "utilities/foldl.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <type_traits>

#define CL_UNIT(cl_type)      \
    struct cl_type##1 final { \
        cl_type s[1];         \
    };

#define CL_FOR_EACH_TYPE(macro)                                         \
    macro(cl_char) macro(cl_uchar) macro(cl_short) macro(cl_ushort)     \
            macro(cl_int) macro(cl_uint) macro(cl_long) macro(cl_ulong) \
                    macro(cl_float) macro(cl_double)

CL_FOR_EACH_TYPE(CL_UNIT)

//  macro machinery ////////////////////////////////////////////////////////////

#define CL_VECTOR_REGISTER_PREFIX(macro, cl_type_prefix_)       \
    macro(cl_type_prefix_##1) macro(cl_type_prefix_##2)         \
            macro(cl_type_prefix_##4) macro(cl_type_prefix_##8) \
                    macro(cl_type_prefix_##16)

#define CL_VECTOR_REGISTER(macro)               \
    CL_VECTOR_REGISTER_PREFIX(macro, cl_char)   \
    CL_VECTOR_REGISTER_PREFIX(macro, cl_uchar)  \
    CL_VECTOR_REGISTER_PREFIX(macro, cl_short)  \
    CL_VECTOR_REGISTER_PREFIX(macro, cl_ushort) \
    CL_VECTOR_REGISTER_PREFIX(macro, cl_int)    \
    CL_VECTOR_REGISTER_PREFIX(macro, cl_uint)   \
    CL_VECTOR_REGISTER_PREFIX(macro, cl_long)   \
    CL_VECTOR_REGISTER_PREFIX(macro, cl_ulong)  \
    CL_VECTOR_REGISTER_PREFIX(macro, cl_float)  \
    CL_VECTOR_REGISTER_PREFIX(macro, cl_double)

namespace detail {

//  find properties, given cl vector type //////////////////////////////////////

template <typename T>
struct cl_vector_type_trait final {
    using is_vector_type = std::false_type;
    using value_type = T;
    using components = std::integral_constant<size_t, 1>;
};

#define DEFINE_CL_VECTOR_TYPE_TRAIT(cl_type)                               \
    template <>                                                            \
    struct cl_vector_type_trait<cl_type> final {                           \
        using is_vector_type = std::true_type;                             \
        using array_type = decltype(std::declval<cl_type>().s);            \
        static_assert(std::rank<array_type>::value == 1,                   \
                      "vector types have array rank 1");                   \
        using value_type = std::remove_all_extents_t<array_type>;          \
        using components =                                                 \
                std::integral_constant<size_t,                             \
                                       std::extent<array_type, 0>::value>; \
    };

CL_VECTOR_REGISTER(DEFINE_CL_VECTOR_TYPE_TRAIT)

template <typename T>
using is_vector_type_t = typename cl_vector_type_trait<T>::is_vector_type;

template <typename T>
constexpr auto is_vector_type_v = is_vector_type_t<T>{};

template <typename T, typename U = void>
using enable_if_is_vector_t = std::enable_if_t<is_vector_type_v<T>, U>;

template <typename T, typename U = void>
using enable_if_is_not_vector_t = std::enable_if_t<!is_vector_type_v<T>, U>;

template <typename T>
using components_t = typename cl_vector_type_trait<T>::components;

template <typename T>
constexpr auto components_v = components_t<T>{};

template <typename T>
using value_type_t = typename cl_vector_type_trait<T>::value_type;

template <typename...>
struct any;

template <typename... Ts>
constexpr auto any_v = any<Ts...>::value;

template <typename T>
struct any<T> final {
    static constexpr auto value = T::value;
};

template <typename T, typename... Ts>
struct any<T, Ts...> final {
    static constexpr auto value = any_v<T> || any_v<Ts...>;
};

template <typename U, typename... Ts>
using enable_if_any_is_vector_t =
        std::enable_if_t<any_v<is_vector_type_t<Ts>...>, U>;

//  constructing from type + size //////////////////////////////////////////////

template <typename T, size_t N>
struct cl_vector_constructor;

#define DEFINE_CL_VECTOR_CONSTRUCTOR_TRAIT(cl_type)                            \
    template <>                                                                \
    struct cl_vector_constructor<value_type_t<cl_type>, components_v<cl_type>> \
            final {                                                            \
        using type = cl_type;                                                  \
    };

CL_VECTOR_REGISTER(DEFINE_CL_VECTOR_CONSTRUCTOR_TRAIT)

template <typename T, size_t N>
using cl_vector_constructor_t = typename cl_vector_constructor<T, N>::type;

template <size_t N>
struct cl_vector_constructor<bool, N> final {
    using type = cl_vector_constructor_t<cl_char, N>;
};

template <typename T, typename Op, enable_if_is_vector_t<T, int> = 0>
constexpr auto accumulate(const T& t, const Op& op) {
    return util::foldl(op, t.s);
}

template <typename T,
          typename Accumulator,
          typename Op,
          enable_if_is_vector_t<T, int> = 0>
constexpr auto accumulate(const T& t,
                          const Accumulator& accumulator,
                          const Op& op) {
    return util::foldl(op, accumulator, t.s);
}

template <typename T, typename U, typename Op, size_t... Ix>
constexpr auto& inplace_zip_both_vector(T& t,
                                        const U& u,
                                        const Op& op,
                                        std::index_sequence<Ix...>) {
    return t = T{{static_cast<value_type_t<T>>(op(t.s[Ix], u.s[Ix]))...}};
}

template <typename T,
          typename U,
          typename Op,
          enable_if_is_vector_t<U, int> = 0>
constexpr auto& inplace_zip(T& t, const U& u, const Op& op) {
    return inplace_zip_both_vector(
            t, u, op, std::make_index_sequence<components_v<T>>{});
}

template <typename T, typename U, typename Op, size_t... Ix>
constexpr auto& inplace_zip_one_vector(T& t,
                                       const U& u,
                                       const Op& op,
                                       std::index_sequence<Ix...>) {
    return t = T{{static_cast<value_type_t<T>>(op(t.s[Ix], u))...}};
}

template <typename T,
          typename U,
          typename Op,
          enable_if_is_not_vector_t<U, int> = 0>
constexpr auto& inplace_zip(T& t, const U& u, const Op& op) {
    return inplace_zip_one_vector(
            t, u, op, std::make_index_sequence<components_v<T>>{});
}

template <typename T, typename U, typename Op, size_t... Ix>
constexpr auto zip_both_vector(const T& t,
                               const U& u,
                               const Op& op,
                               std::index_sequence<Ix...>) {
    using value_type = std::decay_t<decltype(op(t.s[0], u.s[0]))>;
    return cl_vector_constructor_t<value_type, sizeof...(Ix)>{
            {op(t.s[Ix], u.s[Ix])...}};
}

template <typename T,
          typename U,
          typename Op,
          std::enable_if_t<is_vector_type_v<T> && is_vector_type_v<U> &&
                                   components_v<T> == components_v<U>,
                           int> = 0>
constexpr auto zip(const T& t, const U& u, const Op& op) {
    return zip_both_vector(
            t, u, op, std::make_index_sequence<components_v<T>>{});
}

template <typename T, typename U, typename Op, size_t... Ix>
constexpr auto zip_first_vector(const T& t,
                                const U& u,
                                const Op& op,
                                std::index_sequence<Ix...>) {
    using value_type = std::decay_t<decltype(op(t.s[0], u))>;
    return cl_vector_constructor_t<value_type, sizeof...(Ix)>{
            {op(t.s[Ix], u)...}};
}

template <
        typename T,
        typename U,
        typename Op,
        std::enable_if_t<is_vector_type_v<T> && !is_vector_type_v<U>, int> = 0>
constexpr auto zip(const T& t, const U& u, const Op& op) {
    return zip_first_vector(
            t, u, op, std::make_index_sequence<components_v<T>>{});
}

template <typename T, typename U, typename Op, size_t... Ix>
constexpr auto zip_second_vector(const T& t,
                                 const U& u,
                                 const Op& op,
                                 std::index_sequence<Ix...>) {
    using value_type = std::decay_t<decltype(op(t, u.s[0]))>;
    return cl_vector_constructor_t<value_type, sizeof...(Ix)>{
            {op(t, u.s[Ix])...}};
}

template <
        typename T,
        typename U,
        typename Op,
        std::enable_if_t<!is_vector_type_v<T> && is_vector_type_v<U>, int> = 0>
constexpr auto zip(const T& t, const U& u, const Op& op) {
    return zip_second_vector(
            t, u, op, std::make_index_sequence<components_v<U>>{});
}

template <typename T, typename Op, size_t... Ix>
constexpr auto map_impl(const T& t, const Op& op, std::index_sequence<Ix...>) {
    using value_type = std::decay_t<decltype(op(t.s[0]))>;
    return cl_vector_constructor_t<value_type, sizeof...(Ix)>{{op(t.s[Ix])...}};
}

template <typename T, typename Op, enable_if_is_vector_t<T, int> = 0>
constexpr auto map(const T& t, const Op& op) {
    return map_impl(t, op, std::make_index_sequence<components_v<T>>{});
}

}  // namespace detail

//  relational ops

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator==(const T& a, const U& b) {
    return detail::accumulate(detail::zip(a, b, std::equal_to<>{}),
                              std::logical_and<>{});
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto operator!(const T& a) {
    return detail::map(a, std::logical_not<>{});
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator!=(const T& a, const U& b) {
    return !(a == b);
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator<(const T& a, const U& b) {
    return detail::zip(a, b, std::less<>{});
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator<=(const T& a, const U& b) {
    return detail::zip(a, b, std::less_equal<>{});
}

//  arithmetic ops /////////////////////////////////////////////////////////////

template <typename T, typename U, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto& operator+=(T& a, const U& b) {
    return detail::inplace_zip(a, b, std::plus<>{});
}

template <typename T, typename U, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto& operator-=(T& a, const U& b) {
    return detail::inplace_zip(a, b, std::minus<>{});
}

template <typename T, typename U, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto& operator*=(T& a, const U& b) {
    return detail::inplace_zip(a, b, std::multiplies<>{});
}

template <typename T, typename U, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto& operator/=(T& a, const U& b) {
    return detail::inplace_zip(a, b, std::divides<>{});
}

template <typename T, typename U, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto& operator%=(T& a, const U& b) {
    return detail::inplace_zip(a, b, std::modulus<>{});
}

////////////////////////////////////////////////////////////////////////////////

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator+(const T& a, const U& b) {
    return detail::zip(a, b, std::plus<>{});
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator-(const T& a, const U& b) {
    return detail::zip(a, b, std::minus<>{});
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator*(const T& a, const U& b) {
    return detail::zip(a, b, std::multiplies<>{});
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator/(const T& a, const U& b) {
    return detail::zip(a, b, std::divides<>{});
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator%(const T& a, const U& b) {
    return detail::zip(a, b, std::modulus<>{});
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto operator+(const T& a) {
    return a;
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto operator-(const T& a) {
    return detail::map(a, std::negate<>{});
}

//  other reductions ///////////////////////////////////////////////////////////

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto sum(const T& t) {
    return detail::accumulate(t, std::plus<>{});
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto product(const T& t) {
    return detail::accumulate(t, std::multiplies<>{});
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto mean(const T& t) {
    return sum(t) / detail::components_v<T>;
}

//  logical reductions /////////////////////////////////////////////////////////

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr bool all(const T& t) {
    return detail::accumulate(t, std::logical_and<>{});
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr bool any(const T& t) {
    return detail::accumulate(t, std::logical_or<>{});
}

namespace detail {

struct max_functor final {
    template <typename T>
    constexpr const auto& operator()(const T& a, const T& b) const {
        using std::max;
        return max(a, b);
    }
};

struct min_functor final {
    template <typename T>
    constexpr const auto& operator()(const T& a, const T& b) const {
        using std::min;
        return min(a, b);
    }
};

}  // namespace detail

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto min_element(const T& t) {
    return detail::accumulate(t, detail::min_functor{});
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto max_element(const T& t) {
    return detail::accumulate(t, detail::max_functor{});
}

//  misc ///////////////////////////////////////////////////////////////////////

template <typename T,
          size_t components = detail::components_v<T>,
          detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto isnan(const T& t) {
    return detail::map(t, [](auto i) { return std::isnan(i); });
}

template <typename T,
          size_t components = detail::components_v<T>,
          detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto isinf(const T& t) {
    return detail::map(t, [](auto i) { return std::isinf(i); });
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
inline auto sqrt(const T& t) {
    return detail::map(t, [](auto i) { return std::sqrt(i); });
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
inline auto log(const T& t) {
    return detail::map(t, [](auto i) { return std::log(i); });
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
inline auto abs(const T& t) {
    return detail::map(t, [](auto i) { return std::abs(i); });
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
inline auto copysign(const T& t, const U& u) {
    return detail::zip(
            t, u, [](auto i, auto j) { return std::copysign(i, j); });
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
inline auto pow(const T& t, const U& u) {
    return detail::zip(t, u, [](auto i, auto j) { return std::pow(i, j); });
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
inline auto max(const T& t, const U& u) {
    return detail::zip(t, u, detail::max_functor{});
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
inline auto min(const T& t, const U& u) {
    return detail::zip(t, u, detail::min_functor{});
}

template <size_t Components, typename Input>
constexpr auto construct_vector(Input input) {
    detail::cl_vector_constructor_t<Input, Components> ret{};
    for (auto& i : ret.s) {
        i = input;
    }
    return ret;
}
