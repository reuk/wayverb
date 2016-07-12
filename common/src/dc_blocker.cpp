#include "common/dc_blocker.h"

namespace filter {

DelayLine::DelayLine(int length)
        : data(length, 0)
        , index(0) {
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

void DelayLine::clear() {
    std::fill(data.begin(), data.end(), 0);
    index = 0;
}

MovingAverage::MovingAverage(int d)
        : d(d)
        , delay_line(d + 2)
        , single_delay(0) {
}

double MovingAverage::operator()(double x) {
    delay_line.push(x);
    auto ret = x - delay_line[d] + single_delay;
    single_delay = ret;
    return ret / d;
}

void MovingAverage::clear() {
    delay_line.clear();
    single_delay = 0;
}

}  // namespace filter
