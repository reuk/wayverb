#pragma once

#include "glm/glm.hpp"

namespace wayverb {
namespace core {

template <typename T, typename U>
inline bool within_tolerance(T a, U tolerance) {
    return std::abs(a) <= tolerance;
}

template <
        class T,
        typename std::enable_if_t<!std::numeric_limits<T>::is_integer, int> = 0>
constexpr bool almost_equal(T x, T y, size_t ulp) {
    //	from cppreference.com on epsilon
    const auto abs_diff = std::abs(x - y);
    return abs_diff <
                   std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp ||
           abs_diff < std::numeric_limits<T>::min();
}

template <typename T>
inline bool nearby(const T& a, const T& b, double dist) {
    return glm::distance(a, b) <= dist;
}

}  // namespace core
}  // namespace wayverb
