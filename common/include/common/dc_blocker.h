#pragma once

#include "common/aligned/vector.h"
#include "common/filters_common.h"
#include "common/stl_wrappers.h"

namespace filter {

class DelayLine final {
public:
    DelayLine() = default;
    DelayLine(int length);

    double& operator[](size_t i);
    const double& operator[](size_t i) const;
    void push(double x);

    void clear();

private:
    aligned::vector<double> data;
    size_t index;
};

class MovingAverage final {
public:
    MovingAverage() = default;
    MovingAverage(int d);

    double operator()(double x);

    void clear();

    double get_index(size_t i) const;

private:
    int d;
    DelayLine delay_line;
    double single_delay;
};

template <int modules>
class NMovingAverages final {
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

    const MovingAverage& get_averager() const {
        return averages.front();
    }

private:
    std::array<MovingAverage, modules> averages;
};

class LinearDCBlocker final {
public:
    explicit LinearDCBlocker(int d = 128);

    double operator()(double x);

    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
        aligned::vector<float> ret(begin, end);
        std::for_each(std::begin(ret), std::end(ret), [this](auto& i) {
            i = this->operator()(i);
        });
        return ret;
    }

    void clear();

private:
    int d;
    NMovingAverages<2> moving_averages;
};

class ExtraLinearDCBlocker final {
public:
    explicit ExtraLinearDCBlocker(int d = 128);

    double operator()(double x);

    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
        aligned::vector<float> ret(begin, end);
        std::for_each(std::begin(ret), std::end(ret), [this](auto& i) {
            i = this->operator()(i);
        });
        return ret;
    }

    void clear();

private:
    int d;
    DelayLine delay_line;
    NMovingAverages<4> moving_averages;
};

}  // namespace filter
