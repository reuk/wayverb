#pragma once

#include "core/cl/traits.h"
#include "core/exceptions.h"

template <typename T, std::enable_if_t<detail::is_vector_type_v<T>, int> = 0>
constexpr bool is_any_nan(const T& t) {
    return any(isnan(t));
}

template <typename T, std::enable_if_t<!detail::is_vector_type_v<T>, int> = 0>
constexpr bool is_any_nan(T t) {
    return std::isnan(t);
}

////////////////////////////////////////////////////////////////////////////////

template <typename T, std::enable_if_t<detail::is_vector_type_v<T>, int> = 0>
constexpr bool is_any_inf(const T& t) {
    return any(isinf(t));
}

template <typename T, std::enable_if_t<!detail::is_vector_type_v<T>, int> = 0>
constexpr bool is_any_inf(T t) {
    return std::isinf(t);
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
void throw_if_suspicious(const T& t) {
    if (is_any_nan(t)) {
        throw exceptions::value_is_nan(
                "suspicious value found by throw_if_suspicious");
    }

    if (is_any_inf(t)) {
        throw exceptions::value_is_inf(
                "suspicious value found by throw_if_suspicious");
    }
}
