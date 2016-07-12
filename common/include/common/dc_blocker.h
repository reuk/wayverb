#pragma once

#include "common/filters_common.h"
#include "common/stl_wrappers.h"

#include <array>
#include <vector>

namespace filter {

struct DelayLine {
    DelayLine(int length);

    double& operator[](size_t i);
    const double& operator[](size_t i) const;
    void push(double x);

    std::vector<double> data;
    size_t index{0};
};

struct MovingAverage {
    MovingAverage(int d);

    double operator()(double x);

    int d;
    DelayLine delay_line;
    double single_delay{0};
};

struct LinearDCBlocker {
    LinearDCBlocker(int d);

    double operator()(double x);

    template<typename It>
    std::vector<float> filter(It begin, It end) {
        std::vector<float> ret(begin, end);
        std::for_each(std::begin(ret), std::end(ret), [this](auto& i) {
            i = this->operator()(i);
        });
        return ret;
    }

    int d;
    std::array<MovingAverage, 2> moving_average;
};

using ZeroPhaseDCBlocker = TwopassFilterWrapper<LinearDCBlocker>;

}  // namespace filter
