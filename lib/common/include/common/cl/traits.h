#pragma once

#include "common/stl_wrappers.h"

#include "CL/cl.hpp"

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

//  macro machinery ----------------------------------------------------------//

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

//  find properties, given cl vector type ------------------------------------//

template <typename T>
struct cl_vector_type_trait final {
    static constexpr auto is_vector_type = false;
    using value_type = T;
    static constexpr size_t components = 1;
};

#define DEFINE_CL_VECTOR_TYPE_TRAIT(cl_type)                                  \
    template <>                                                               \
    struct cl_vector_type_trait<cl_type> final {                              \
        static constexpr auto is_vector_type = true;                          \
        using array_type = decltype(std::declval<cl_type>().s);               \
        static_assert(std::rank<array_type>::value == 1,                      \
                      "vector types have array rank 1");                      \
        using value_type = std::remove_all_extents_t<array_type>;             \
        static constexpr auto components = std::extent<array_type, 0>::value; \
    };

CL_VECTOR_REGISTER(DEFINE_CL_VECTOR_TYPE_TRAIT)

template <typename T>
constexpr auto is_vector_type_v = cl_vector_type_trait<T>::is_vector_type;

template <typename T>
using is_vector_type_t = std::integral_constant<bool, is_vector_type_v<T>>;

template <typename T, typename U = void>
using enable_if_is_vector_t = std::enable_if_t<is_vector_type_v<T>, U>;

template <typename T, typename U = void>
using enable_if_is_not_vector_t = std::enable_if_t<!is_vector_type_v<T>, U>;

template <typename T>
constexpr auto components_v = cl_vector_type_trait<T>::components;

template <typename T>
using value_type_t = typename cl_vector_type_trait<T>::value_type;

template<typename...>
struct any;

template <typename... Ts>
constexpr auto any_v{any<Ts...>::value};

template <typename T>
struct any<T> final {
    static constexpr auto value{T::value};
};

template <typename T, typename... Ts>
struct any<T, Ts...> final {
    static constexpr auto value{any_v<T> || any_v<Ts...>};
};

template <typename U, typename... Ts>
using enable_if_any_is_vector_t =
        std::enable_if_t<any_v<is_vector_type_t<Ts>...>, U>;

//  constructing from type + size --------------------------------------------//

template <typename T, size_t N>
struct cl_vector_constructor;

#define DEFINE_CL_VECTOR_CONSTRUCTOR_TRAIT(cl_type)             \
    template <>                                                 \
    struct cl_vector_constructor<                               \
            typename cl_vector_type_trait<cl_type>::value_type, \
            cl_vector_type_trait<cl_type>::components>          \
            final {                                             \
        using type = cl_type;                                   \
    };

CL_VECTOR_REGISTER(DEFINE_CL_VECTOR_CONSTRUCTOR_TRAIT)

template <typename T, size_t N>
using cl_vector_constructor_t = typename cl_vector_constructor<T, N>::type;

template <size_t N>
struct cl_vector_constructor<bool, N> final {
    using type = cl_vector_constructor_t<cl_char, N>;
};

template <typename Ret, typename Input>
constexpr auto construct_vector(Input input) {
    Ret ret{};
    for (auto& i : ret.s) {
        i = input;
    }
    return ret;
}

//  interesting arithmetic ops -----------------------------------------------//

namespace accumulator {

struct stop final {
    using type = stop;

    template <typename... Ts>
    constexpr explicit stop(Ts&&... ts) {}
};

template <typename T, size_t index>
struct accumulatable_vector;

template <typename T, size_t index>
struct next final {
    using type = accumulatable_vector<T, index - 1>;
};

template <typename T>
struct next<T, 0> final {
    using type = stop;
};

template <typename T, size_t index>
using next_t = typename next<T, index>::type;

template <typename T, size_t index = components_v<T> - 1>
struct accumulatable_vector final {
    using value_type = T;
    constexpr accumulatable_vector(const value_type& value)
            : value(value) {}
    const value_type& value;

    constexpr auto current() const { return value.s[index]; }
    constexpr next_t<T, index> next() const { return next_t<T, index>(value); }
};

template <typename Accumulator, typename Op>
constexpr auto impl(stop, const Accumulator& accumulator, Op op) {
    return accumulator;
}

template <typename T, typename Accumulator, typename Op>
constexpr auto impl(const T& t, const Accumulator& accumulator, Op op) {
    return impl(t.next(), op(accumulator, t.current()), op);
}

}  // namespace accumulator

template <typename T,
          typename Accumulator,
          typename Op,
          enable_if_is_vector_t<T, int> = 0>
constexpr auto accumulate(const T& t, const Accumulator& accumulator, Op op) {
    return accumulator::impl(
            accumulator::accumulatable_vector<T>(t), accumulator, op);
}

template <typename T,
          typename U,
          typename Op,
          enable_if_is_vector_t<U, int> = 0>
constexpr auto& inplace_zip(T& t, const U& u, Op op) {
    using std::begin;
    using std::end;
    auto i = begin(t.s);
    auto j = begin(u.s);
    for (; i != end(t.s); ++i, ++j) {
        *i = op(*i, *j);
    }
    return t;
}

template <typename T,
          typename U,
          typename Op,
          enable_if_is_not_vector_t<U, int> = 0>
constexpr auto& inplace_zip(T& t, const U& u, Op op) {
    using std::begin;
    using std::end;
    auto i = begin(t.s);
    for (; i != end(t.s); ++i) {
        *i = op(*i, u);
    }
    return t;
}

template <typename T,
          typename U,
          typename Op,
          std::enable_if_t<is_vector_type_v<T> && is_vector_type_v<U> &&
                                   components_v<T> == components_v<U>,
                           int> = 0>
constexpr auto zip(const T& t, const U& u, Op op) {
    using value_type = decltype(op(t.s[0], u.s[0]));

    cl_vector_constructor_t<value_type, components_v<T>> ret{};

    using std::begin;
    using std::end;

    auto i = begin(t.s);
    auto j = begin(u.s);
    auto k = begin(ret.s);
    for (; i != end(t.s); ++i, ++j, ++k) {
        *k = op(*i, *j);
    }

    return ret;
}

template <
        typename T,
        typename U,
        typename Op,
        std::enable_if_t<is_vector_type_v<T> && !is_vector_type_v<U>, int> = 0>
constexpr auto zip(const T& t, U u, Op op) {
    return zip(t,
               construct_vector<cl_vector_constructor_t<U, components_v<T>>>(u),
               op);
}

template <
        typename T,
        typename U,
        typename Op,
        std::enable_if_t<!is_vector_type_v<T> && is_vector_type_v<U>, int> = 0>
constexpr auto zip(T t, const U& u, Op op) {
    return zip(construct_vector<cl_vector_constructor_t<T, components_v<U>>>(t),
               u,
               op);
}

template <typename T, typename Op, enable_if_is_vector_t<T, int> = 0>
constexpr auto& for_each(T& t, Op op) {
    for (auto& i : t.s) {
        i = op(i);
    }
    return t;
}

template <typename T, typename Op, enable_if_is_vector_t<T, int> = 0>
constexpr auto map(const T& t, Op op) {
    using value_type = decltype(op(t.s[0]));

    cl_vector_constructor_t<value_type, components_v<T>> ret{};

    using std::begin;
    using std::end;
    auto i{begin(t.s)};
    auto j{begin(ret.s)};
    for (; i != end(t.s); ++i, ++j) {
        *j = op(*i);
    }

    return ret;
}

//  conversions probably -----------------------------------------------------//

template <typename T, typename U, enable_if_is_vector_t<U, int> = 0>
constexpr T convert(const U& u) {
    struct conversion final {
        constexpr value_type_t<T> operator()(const value_type_t<U>& u) const {
            return u;
        }
    };
    return map(u, conversion{});
}

template <typename T, typename U, enable_if_is_not_vector_t<U, int> = 0>
constexpr T convert(const U& u) {
    return construct_vector<T>(u);
}

template <typename... Ts>
using common_value_t = std::common_type_t<value_type_t<Ts>...>;

template <typename...>
struct common_size;

template <typename... Ts>
constexpr auto common_size_v{common_size<Ts...>::value};

template <typename T>
struct common_size<T> final {
    static constexpr auto value{components_v<T>};
};

template <typename T, typename... Ts>
struct common_size<T, Ts...> final {
    static_assert(common_size_v<T> == common_size_v<Ts...> ||
                          common_size_v<T> == 1 || common_size_v<Ts...> == 1,
                  "size must be 1, or match the other size");
    static constexpr auto value{
            std::max(common_size_v<T>, common_size_v<Ts...>)};
};

template <typename... Ts>
using common_vector_t =
        cl_vector_constructor_t<common_value_t<Ts...>, common_size_v<Ts...>>;

}  // namespace detail

//  relational ops

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator==(const T& a, const U& b) {
    //    return detail::zip(a, b, std::equal_to<>());
    using common_t = detail::common_vector_t<T, U>;
    return detail::accumulate(detail::zip(detail::convert<common_t>(a),
                                          detail::convert<common_t>(b),
                                          std::equal_to<>()),
                              true,
                              std::logical_and<>());
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto operator!(const T& a) {
    return detail::map(a, std::logical_not<>());
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
    using common_t = detail::common_vector_t<T, U>;
    return detail::zip(detail::convert<common_t>(a),
                       detail::convert<common_t>(b),
                       std::less<>());
}

//  arithmetic ops -----------------------------------------------------------//

template <typename T, typename U, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto& operator+=(T& a, const U& b) {
    return detail::inplace_zip(a, b, std::plus<>());
}

template <typename T, typename U, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto& operator-=(T& a, const U& b) {
    return detail::inplace_zip(a, b, std::minus<>());
}

template <typename T, typename U, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto& operator*=(T& a, const U& b) {
    return detail::inplace_zip(a, b, std::multiplies<>());
}

template <typename T, typename U, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto& operator/=(T& a, const U& b) {
    return detail::inplace_zip(a, b, std::divides<>());
}

template <typename T, typename U, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto& operator%=(T& a, const U& b) {
    return detail::inplace_zip(a, b, std::modulus<>());
}

//----------------------------------------------------------------------------//

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator+(const T& a, const U& b) {
    return detail::zip(a, b, std::plus<>());
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator-(const T& a, const U& b) {
    return detail::zip(a, b, std::minus<>());
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator*(const T& a, const U& b) {
    return detail::zip(a, b, std::multiplies<>());
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator/(const T& a, const U& b) {
    return detail::zip(a, b, std::divides<>());
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
constexpr auto operator%(const T& a, const U& b) {
    return detail::zip(a, b, std::modulus<>());
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto operator+(const T& a) {
    return a;
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto operator-(const T& a) {
    return detail::map(a, std::negate<>());
}

//  other reductions ---------------------------------------------------------//

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto sum(const T& t) {
    return detail::accumulate(t, 0, std::plus<>());
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto product(const T& t) {
    return detail::accumulate(t, 1, std::multiplies<>());
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto mean(const T& t) {
    return sum(t) / detail::components_v<T>;
}

//  logical reductions -------------------------------------------------------//

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr bool all(const T& t) {
    auto ret{true};
    for (const auto i : t.s) {
        ret = ret && i;
    }
    return ret;
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr bool any(const T& t) {
    auto ret{false};
    for (const auto i : t.s) {
        ret = ret || i;
    }
    return ret;
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto min(const T& t) {
    using value_type = detail::value_type_t<T>;
    struct min final {
        constexpr value_type operator()(value_type a, value_type b) const {
            using std::min;
            return min(a, b);
        }
    };
    return detail::accumulate(t, std::numeric_limits<value_type>::max(), min{});
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto max(const T& t) {
    using value_type = detail::value_type_t<T>;
    struct max final {
        constexpr value_type operator()(value_type a, value_type b) const {
            using std::max;
            return max(a, b);
        }
    };
    return detail::accumulate(
            t, std::numeric_limits<value_type>::lowest(), max{});
}

//  misc ---------------------------------------------------------------------//

namespace detail {

template <typename T,
          typename Func,
          size_t components = detail::components_v<T>,
          detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto map_func_to_bool(const T& t, Func f) {
    using Ret = detail::cl_vector_constructor_t<bool, components>;
    Ret ret{};
    proc::transform(t.s, ret.s, [&](auto i) { return f(i); });
    return ret;
}

}  // namespace detail

template <typename T,
          size_t components = detail::components_v<T>,
          detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto isnan(const T& t) {
    return detail::map_func_to_bool(t, [](auto i) { return std::isnan(i); });
}

template <typename T,
          size_t components = detail::components_v<T>,
          detail::enable_if_is_vector_t<T, int> = 0>
constexpr auto isinf(const T& t) {
    return detail::map_func_to_bool(t, [](auto i) { return std::isinf(i); });
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
inline auto sqrt(const T& t) {
    return detail::map(t, [](auto i) {
        using std::sqrt;
        return sqrt(i);
    });
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
inline auto log(const T& t) {
    return detail::map(t, [](auto i) {
        using std::log;
        return log(i);
    });
}

template <typename T, detail::enable_if_is_vector_t<T, int> = 0>
inline auto abs(const T& t) {
    return detail::map(t, [](auto i) {
        using std::abs;
        return abs(i);
    });
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
inline auto copysign(const T& t, const U& u) {
    using common_t = detail::common_vector_t<T, U>;
    using std::copysign;
    return detail::zip(detail::convert<common_t>(t),
                       detail::convert<common_t>(u),
                       [](auto i, auto j) { return copysign(i, j); });
}

template <typename T,
          typename U,
          detail::enable_if_any_is_vector_t<int, T, U> = 0>
inline auto pow(const T& t, const U& u) {
    using common_t = detail::common_vector_t<T, U>;
    using std::pow;
    return detail::zip(detail::convert<common_t>(t),
                       detail::convert<common_t>(u),
                       [](auto i, auto j) { return pow(i, j); });
}
