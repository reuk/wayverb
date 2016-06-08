#pragma once

#include "cl_include.h"
#include "extended_algorithms.h"
#include "reduce.h"

#include <functional>

namespace cl_math {

namespace detail {

template <typename T>
struct NumElementsTrait;

template <>
struct NumElementsTrait<cl_float2> : public std::integral_constant<size_t, 2> {
};
template <>
struct NumElementsTrait<cl_float4> : public std::integral_constant<size_t, 4> {
};
template <>
struct NumElementsTrait<cl_float8> : public std::integral_constant<size_t, 8> {
};
template <>
struct NumElementsTrait<cl_float16>
        : public std::integral_constant<size_t, 16> {};

template <>
struct NumElementsTrait<cl_int2> : public std::integral_constant<size_t, 2> {};
template <>
struct NumElementsTrait<cl_int4> : public std::integral_constant<size_t, 4> {};
template <>
struct NumElementsTrait<cl_int8> : public std::integral_constant<size_t, 8> {};
template <>
struct NumElementsTrait<cl_int16> : public std::integral_constant<size_t, 16> {
};

template <typename T>
struct ElementTypeTrait;

template <>
struct ElementTypeTrait<cl_float2> {
    using type = float;
};
template <>
struct ElementTypeTrait<cl_float4> {
    using type = float;
};
template <>
struct ElementTypeTrait<cl_float8> {
    using type = float;
};
template <>
struct ElementTypeTrait<cl_float16> {
    using type = float;
};
template <>
struct ElementTypeTrait<cl_int2> {
    using type = int;
};
template <>
struct ElementTypeTrait<cl_int4> {
    using type = int;
};
template <>
struct ElementTypeTrait<cl_int8> {
    using type = int;
};
template <>
struct ElementTypeTrait<cl_int16> {
    using type = int;
};

template <typename T, size_t ELEMENTS>
struct VectorTypeTrait;

template <>
struct VectorTypeTrait<float, 2> {
    using type = cl_float2;
};
template <>
struct VectorTypeTrait<float, 4> {
    using type = cl_float4;
};
template <>
struct VectorTypeTrait<float, 8> {
    using type = cl_float8;
};
template <>
struct VectorTypeTrait<float, 16> {
    using type = cl_float16;
};
template <>
struct VectorTypeTrait<int, 2> {
    using type = cl_int2;
};
template <>
struct VectorTypeTrait<int, 4> {
    using type = cl_int4;
};
template <>
struct VectorTypeTrait<int, 8> {
    using type = cl_int8;
};
template <>
struct VectorTypeTrait<int, 16> {
    using type = cl_int16;
};

template <typename T, size_t ELEMENTS, size_t... Ix>
constexpr typename VectorTypeTrait<T, ELEMENTS>::type convert(
        std::index_sequence<Ix...>, const std::array<T, ELEMENTS>& t) {
    return typename VectorTypeTrait<T, ELEMENTS>::type{{t[Ix]...}};
}

template <typename T, size_t ELEMENTS>
constexpr auto convert(const std::array<T, ELEMENTS>& t) {
    return convert(std::make_index_sequence<ELEMENTS>(), t);
}

template <size_t I, typename T, typename... U>
constexpr auto zip_row(const T& t, U&&... u) {
    return std::make_tuple(t.s[I], (u.s[I])...);
}

template <typename T, typename... U, size_t... Ix>
constexpr auto zip(std::index_sequence<Ix...>, const T& t, U&&... u) {
    constexpr auto num_elements = NumElementsTrait<T>::value;
    return std::array<decltype(zip_row<0>(t, u...)), num_elements>{
            {zip_row<Ix>(t, std::forward<U>(u)...)...}};
}

}  // namespace detail

template <typename T, typename... U>
constexpr auto zip(const T& t, U&&... u) {
    constexpr auto num_elements = detail::NumElementsTrait<T>::value;
    return detail::zip(
            std::make_index_sequence<num_elements>(), t, std::forward<U>(u)...);
}

template <typename Func, typename T, typename... U>
constexpr auto apply(const Func& func, const T& t, U&&... u) {
    return proc::map(zip(t, u...), func);
}

}  // namespace cl_math

#define CL_VEC_OP(sym, functor)                                            \
    template <typename T>                                                  \
    constexpr auto operator sym(const T& a, const T& b) {                  \
        using functor_type = typename std::functor<                        \
                typename cl_math::detail::ElementTypeTrait<T>::type>;      \
        return cl_math::detail::convert(cl_math::apply(                    \
                proc::InvokeFunctor<functor_type>(functor_type()), a, b)); \
    }

CL_VEC_OP(+, plus);
CL_VEC_OP(-, minus);
CL_VEC_OP(*, multiplies);
CL_VEC_OP(/, divides);
CL_VEC_OP(%, modulus);

template <typename T>
constexpr bool operator==(const T& a, const T& b) {
    using functor_type =
            std::equal_to<typename cl_math::detail::ElementTypeTrait<T>::type>;
    return all(cl_math::apply(
            proc::InvokeFunctor<functor_type>(functor_type()), a, b));
}
