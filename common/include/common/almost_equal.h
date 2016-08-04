#pragma once

#include "glm/glm.hpp"

template<typename T, typename U>
inline bool within_tolerance(T a, U tolerance) {
    return std::abs(a) <= tolerance;
}

template <typename T>
inline bool almost_equal(T a, T b, double tolerance) {
    return within_tolerance(a - b, tolerance);
}

template <typename T>
inline bool almost_equal(T a, T b, size_t ups) {
    return almost_equal(a,
                        b,
                        std::numeric_limits<T>::epsilon() *
                                std::max(std::abs(a), std::abs(b)) * ups);
}

template <typename T>
inline bool nearby(const T& a, const T& b, double dist) {
    return glm::distance(a, b) <= dist;
}
