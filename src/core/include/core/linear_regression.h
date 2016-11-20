#pragma once

#include <cmath>
#include <numeric>
#include <stdexcept>

namespace wayverb {
namespace core {
struct linear_regression final {
    double m;  /// gradient of regression line
    double c;  /// y intercept
    double r;  /// product-moment correlation coefficient
};

template <typename It>
linear_regression simple_linear_regression(It begin, It end) {
    const auto n = std::distance(begin, end);

    if (!n) {
        throw std::runtime_error(
                "Empty range passed to simple linear regression.");
    }

    const auto sx =
            std::accumulate(begin, end, 0.0, [](auto running_total, auto i) {
                return running_total + i.x;
            });
    const auto sy =
            std::accumulate(begin, end, 0.0, [](auto running_total, auto i) {
                return running_total + i.y;
            });
    const auto sxx =
            std::accumulate(begin, end, 0.0, [](auto running_total, auto i) {
                return running_total + i.x * i.x;
            });
    const auto sxy =
            std::accumulate(begin, end, 0.0, [](auto running_total, auto i) {
                return running_total + i.x * i.y;
            });
    const auto syy =
            std::accumulate(begin, end, 0.0, [](auto running_total, auto i) {
                return running_total + i.y * i.y;
            });

    const auto denominator = n * sxx - sx * sx;
    if (denominator == 0.0) {
        throw std::runtime_error(
                "Linear regression estimated a denominator of 0");
    }

    const auto numerator = n * sxy - sx * sy;
    const auto m = numerator / denominator;
    const auto c = sy / n - m * sx / n;
    const auto r =
            numerator / std::sqrt((n * sxx - sx * sx) * (n * syy - sy * sy));
    return {m, c, r};
}
}  // namespace core
}  // namespace wayverb
