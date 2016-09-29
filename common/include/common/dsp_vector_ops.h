#pragma once

#include "stl_wrappers.h"

#include <cmath>
#include <numeric>
#include <vector>

inline double recursive_sum(float t) { return t; }
inline double recursive_sum(double t) { return t; }

template <typename T>
inline auto recursive_sum(const T& t) {
    return proc::accumulate(t, 0.0, [](const auto& a, const auto& b) {
        return a + recursive_sum(b);
    });
}

template <typename T>
inline auto count(const T& t) {
    return proc::accumulate(
            t, 0u, [](const auto& a, const auto& b) { return a + count(b); });
}

template <typename T, typename Allocator>
inline auto count(const std::vector<T, Allocator>& coll) {
    return coll.size();
}

template <typename T>
inline auto mean(const T& t) {
    return recursive_sum(t) / count(t);
}

inline double max_mag(float t) { return std::fabs(t); }
inline double max_mag(double t) { return std::fabs(t); }

template <typename T>
inline auto max_mag(const T& t) {
    return proc::accumulate(
            t, 0.0, [](double a, auto b) { return std::max(a, max_mag(b)); });
}

/// The base case of the mul recursion.
inline void mul(float& ret, double f) { ret *= f; }
inline void mul(double& ret, double f) { ret *= f; }

/// Recursively multiply by reference.
template <typename T>
inline void mul(T& ret, double f) {
    for (auto& i : ret) {
        mul(i, f);
    }
}

/// Find the largest absolute value in an arbitarily nested vector, then
/// divide every item in the vector by that value.
template <typename T>
inline void normalize(T& ret) {
    if (const auto mag{max_mag(ret)}) {
        mul(ret, 1.0 / mag);
    }
}

template <typename T>
inline void kernel_normalize(T& ret) {
    const auto sum{std::abs(proc::accumulate(ret, 0.0))};
    if (sum) {
        mul(ret,
            1.0 / sum - std::numeric_limits<typename T::value_type>::epsilon());
    }
}
