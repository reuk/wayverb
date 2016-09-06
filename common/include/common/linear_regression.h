#pragma once

#include <numeric>
#include <stdexcept>

struct linear_regression final {
    double m;  /// gradient of regression line
    double c;  /// y intercept
};

template <typename It>
linear_regression simple_linear_regression(It begin, It end) {
    const auto n{std::distance(begin, end)};

    if (!n) {
        throw std::runtime_error("can't find regression of empty range");
    }

    const auto sx{
            std::accumulate(begin, end, 0.0, [](auto running_total, auto i) {
                return running_total + i.x;
            })};
    const auto sy{
            std::accumulate(begin, end, 0.0, [](auto running_total, auto i) {
                return running_total + i.y;
            })};
    const auto sxx{
            std::accumulate(begin, end, 0.0, [](auto running_total, auto i) {
                return running_total + i.x * i.x;
            })};
    const auto sxy{
            std::accumulate(begin, end, 0.0, [](auto running_total, auto i) {
                return running_total + i.x * i.y;
            })};

    const auto denominator{n * sxx - sx * sx};
    if (!denominator) {
        throw std::runtime_error("regression test: denominator of 0");
    }

    const auto m{(n * sxy - sx * sy) / denominator};
    const auto c{sy / n - m * sx / n};
    return {m, c};
}
