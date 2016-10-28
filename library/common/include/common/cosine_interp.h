#pragma once

#include "utilities/mapping_iterator_adapter.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

/// x is between 0 and 1
/// a is a value
/// b is another value
///
/// If x == 0, output is a; if x == 1, output is b
template <typename U>
auto linear_interp(double x, U a, U b) {
    return a + x * (b - a);
}

struct linear_interp_functor final {
    template <typename U>
    auto operator()(double x, U a, U b) const {
        return linear_interp(x, a, b);
    }
};

/// x is between 0 and 1
/// a is a value
/// b is another value
template <typename U>
auto cosine_interp(double x, U a, U b) {
    return linear_interp(std::cos(M_PI * x) * 0.5 + 0.5, b, a);
}

struct cosine_interp_functor final {
    template <typename U>
    auto operator()(double x, U a, U b) const {
        return cosine_interp(x, a, b);
    }
};

template <typename T, typename U, typename Func>
auto interp(double a, T x1, T x2, U y1, U y2, Func&& func) {
    return func((a - x1) / (x2 - x1), y1, y2);
}

/// In the case where we want to interpolate a one-off value, binary-search is
/// fastest.
template <typename It, typename Func>
auto interp(It b, It e, double a, Func&& func) {
    if (b == e) {
        throw std::runtime_error{"can't interpolate empty range"};
    }

    const auto it =
            std::lower_bound(b, e, a, [&](const auto& i, const auto& j) {
                return i.x < j;
            });

    if (it == b) {
        auto tmp = *b;
        return tmp.y;
    }

    if (it == e) {
        auto tmp = *(e - 1);
        return tmp.y;
    }

    const auto a1 = *(it - 1);
    const auto a2 = *it;

    return interp(a, a1.x, a2.x, a1.y, a2.y, std::forward<Func>(func));
}
