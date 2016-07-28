#include "common/dc_blocker.h"

namespace filter {

DelayLine::DelayLine(int length)
        : data(length, 0)
        , index(0) {}

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

void DelayLine::clear() {
    std::fill(data.begin(), data.end(), 0);
    index = 0;
}

MovingAverage::MovingAverage(int d)
        : d(d)
        , delay_line(d + 2)
        , single_delay(0) {}

double MovingAverage::operator()(double x) {
    delay_line.push(x);
    auto ret     = x - delay_line[d] + single_delay;
    single_delay = ret;
    return ret / d;
}

void MovingAverage::clear() {
    delay_line.clear();
    single_delay = 0;
}

double MovingAverage::get_index(size_t i) const { return delay_line[i]; }

LinearDCBlocker::LinearDCBlocker(int d)
        : d(d)
        , moving_averages(d) {}

double LinearDCBlocker::operator()(double x) {
    x = moving_averages(x);
    return moving_averages.get_averager().get_index(d - 1) - x;
}

void LinearDCBlocker::clear() { moving_averages.clear(); }

ExtraLinearDCBlocker::ExtraLinearDCBlocker(int d)
        : d(d)
        , delay_line(d + 2)
        , moving_averages(d) {}

double ExtraLinearDCBlocker::operator()(double x) {
    x = moving_averages(x);
    delay_line.push(moving_averages.get_averager().get_index(d - 1));
    return delay_line[d - 1] - x;
}

void ExtraLinearDCBlocker::clear() {
    delay_line.clear();
    moving_averages.clear();
}

}  // namespace filter
