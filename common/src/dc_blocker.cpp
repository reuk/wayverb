#include "common/dc_blocker.h"

namespace filter {

DelayLine::DelayLine(int length)
        : data(length, 0) {
}

double& DelayLine::operator[](size_t i) {
    return data[(i + index) % data.size()];
}

const double& DelayLine::operator[](size_t i) const {
    return data[(i + index) % data.size()];
}

void DelayLine::push(double x) {
    if (index == 0) {
        index = data.size();
    }

    index -= 1;
    data[index] = x;
}

MovingAverage::MovingAverage(int d)
        : d(d)
        , delay_line(d + 2) {
}

double MovingAverage::operator()(double x) {
    auto ret = x - delay_line[d - 1];
    delay_line.push(x);
    ret += single_delay;
    single_delay = ret;
    return ret / d;
}

LinearDCBlocker::LinearDCBlocker(int d)
        : d(d)
        , moving_average{{MovingAverage(d), MovingAverage(d)}} {
}

double LinearDCBlocker::operator()(double x) {
    for (auto& m : moving_average) {
        x = m(x);
    }
    return moving_average[0].delay_line[d - 1] - x;
}

}  // namespace filter
