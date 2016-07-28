#pragma once

#include "common/cl_include.h"

#include <functional>
#include <type_traits>

template <typename T>
struct cl_vector_descriptor {
    using array_type = decltype(std::declval<T>().s);
    static_assert(std::rank<array_type>::value == 1,
                  "vector types have array rank 1");
    using value_type                 = std::remove_all_extents_t<array_type>;
    static constexpr auto components = std::extent<array_type, 0>::value;

protected:
    ~cl_vector_descriptor() noexcept = default;  //  don't instantiate this shit
};

template <typename T>
struct cl_vector_type_trait;

#define DEFINE_CL_VECTOR_TYPE_TRAIT(cl_type)         \
    template <>                                      \
    struct cl_vector_type_trait<cl_type> final       \
            : public cl_vector_descriptor<cl_type> { \
        static constexpr auto is_vector_type = true; \
    };

#define DEFINE_CL_VECTOR_TYPE_TRAIT_PREFIX(cl_type_prefix_) \
    DEFINE_CL_VECTOR_TYPE_TRAIT(cl_type_prefix_##2)         \
    DEFINE_CL_VECTOR_TYPE_TRAIT(cl_type_prefix_##4)         \
    DEFINE_CL_VECTOR_TYPE_TRAIT(cl_type_prefix_##8)         \
    DEFINE_CL_VECTOR_TYPE_TRAIT(cl_type_prefix_##16)

//  wew lad

DEFINE_CL_VECTOR_TYPE_TRAIT_PREFIX(cl_char)
DEFINE_CL_VECTOR_TYPE_TRAIT_PREFIX(cl_uchar)
DEFINE_CL_VECTOR_TYPE_TRAIT_PREFIX(cl_short)
DEFINE_CL_VECTOR_TYPE_TRAIT_PREFIX(cl_ushort)
DEFINE_CL_VECTOR_TYPE_TRAIT_PREFIX(cl_int)
DEFINE_CL_VECTOR_TYPE_TRAIT_PREFIX(cl_uint)
DEFINE_CL_VECTOR_TYPE_TRAIT_PREFIX(cl_long)
DEFINE_CL_VECTOR_TYPE_TRAIT_PREFIX(cl_ulong)
DEFINE_CL_VECTOR_TYPE_TRAIT_PREFIX(cl_float)
DEFINE_CL_VECTOR_TYPE_TRAIT_PREFIX(cl_double)

//  interesting arithmetic ops

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr bool operator==(const T& a, const T& b) {
    using std::begin;
    using std::end;
    return std::equal(begin(a.s), end(a.s), begin(b.s));
}

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr bool operator!=(const T& a, const T& b) {
    return !(a == b);
}

namespace detail {
template <typename T, typename Op>
constexpr T& cl_inplace_zip(T& a, const T& b, Op op) {
    using std::begin;
    using std::end;
    auto i = begin(a.s);
    auto j = begin(b.s);
    for (; i != end(a.s); ++i, ++j) {
        *i = op(*i, *j);
    }
    return a;
}
}  // namespace detail

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr T& operator+=(T& a, const T& b) {
    return detail::cl_inplace_zip(a, b, std::plus<>());
}

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr T& operator-=(T& a, const T& b) {
    return detail::cl_inplace_zip(a, b, std::minus<>());
}

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr T& operator*=(T& a, const T& b) {
    return detail::cl_inplace_zip(a, b, std::multiplies<>());
}

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr T& operator/=(T& a, const T& b) {
    return detail::cl_inplace_zip(a, b, std::divides<>());
}

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr T operator+(const T& a, const T& b) {
    auto ret = a;
    return ret += b;
}

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr T operator-(const T& a, const T& b) {
    auto ret = a;
    return ret -= b;
}

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr T operator*(const T& a, const T& b) {
    auto ret = a;
    return ret *= b;
}

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr T operator/(const T& a, const T& b) {
    auto ret = a;
    return ret /= b;
}

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr T operator+(const T& a) {
    return a;
}

template <typename T, typename = typename cl_vector_type_trait<T>::value_type>
constexpr T operator-(const T& a) {
    auto ret = a;
    for (auto& i : ret.s) {
        i = -i;
    }
    return ret;
}
