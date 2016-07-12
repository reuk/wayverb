#pragma once

#include "common/filters_common.h"
#include "common/stl_wrappers.h"

#include <array>
#include <vector>

namespace filter {

class DelayLine {
public:
    DelayLine() = default;
    DelayLine(int length);

    double& operator[](size_t i);
    const double& operator[](size_t i) const;
    void push(double x);

    void clear();

    std::vector<double> data;
    size_t index;
};

class MovingAverage {
public:
    MovingAverage() = default;
    MovingAverage(int d);

    double operator()(double x);

    void clear();

    int d;
    DelayLine delay_line;
    double single_delay;
};

template <int modules>
class NMovingAverages {
public:
    NMovingAverages() = default;
    NMovingAverages(int d) {
        std::fill(averages.begin(), averages.end(), MovingAverage(d));
    }

    double operator()(double x) {
        for (auto& i : averages) {
            x = i(x);
        }
        return x;
    }

    void clear() {
        for (auto& i : averages) {
            i.clear();
        }
    }

    std::array<MovingAverage, modules> averages;
};

class LinearDCBlocker {
public:
    explicit LinearDCBlocker(int d = 128)
            : d(d)
            , moving_averages(d) {
    }

    double operator()(double x) {
        x = moving_averages(x);
        return moving_averages.averages[0].delay_line[d - 1] - x;
    }

    template <typename It>
    std::vector<float> filter(It begin, It end) {
        std::vector<float> ret(begin, end);
        std::for_each(std::begin(ret), std::end(ret), [this](auto& i) {
            i = this->operator()(i);
        });
        return ret;
    }

    void clear() {
        moving_averages.clear();
    }

    int d;
    NMovingAverages<2> moving_averages;
};

class ExtraLinearDCBlocker {
public:
    explicit ExtraLinearDCBlocker(int d = 128)
            : d(d)
            , delay_line(d + 2)
            , moving_averages(d) {
    }

    double operator()(double x) {
        x = moving_averages(x);
        delay_line.push(moving_averages.averages[0].delay_line[d - 1]);
        return delay_line[d - 1] - x;
    }

    template <typename It>
    std::vector<float> filter(It begin, It end) {
        std::vector<float> ret(begin, end);
        std::for_each(std::begin(ret), std::end(ret), [this](auto& i) {
            i = this->operator()(i);
        });
        return ret;
    }

    void clear() {
        delay_line.clear();
        moving_averages.clear();
    }

    int d;
    DelayLine delay_line;
    NMovingAverages<4> moving_averages;
};

}  // namespace filter
