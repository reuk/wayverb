#pragma once

#include "common/stl_wrappers.h"

#include <array>
#include <vector>

namespace filter {

struct DelayLine final {
    DelayLine(int length);

    double& operator[](size_t i);
    const double& operator[](size_t i) const;
    void push(double x);

    std::vector<double> data;
    size_t index{0};
};

struct MovingAverage final {
    MovingAverage(int d);

    double operator()(double x);

    int d;
    DelayLine delay_line;
    double single_delay{0};
};

struct LinearDCBlocker final {
    LinearDCBlocker(int d);

    double operator()(double x);

    template <typename T>
    void filter(T& t) {
        proc::for_each(t, [this](auto& i) { i = this->operator()(i); });
    }

    int d;
    std::array<MovingAverage, 2> moving_average;
};

}  // namespace filter
