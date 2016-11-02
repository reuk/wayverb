#include "core/dc_blocker.h"

namespace filter {

delay_line::delay_line(int length)
        : data(length, 0)
        , index(0) {}

double& delay_line::operator[](size_t i) {
    return data[(i + index) % data.size()];
}

const double& delay_line::operator[](size_t i) const {
    return data[(i + index) % data.size()];
}

void delay_line::push(double x) {
    if (index == 0) {
        index = data.size();
    }

    index -= 1;
    data[index] = x;
}

void delay_line::clear() {
    std::fill(data.begin(), data.end(), 0);
    index = 0;
}

moving_average::moving_average(int d)
        : d(d)
        , delay_line(d + 2)
        , single_delay(0) {}

double moving_average::filter(double x) {
    delay_line.push(x);
    auto ret = x - delay_line[d] + single_delay;
    single_delay = ret;
    return ret / d;
}

void moving_average::clear() {
    delay_line.clear();
    single_delay = 0;
}

double moving_average::get_index(size_t i) const { return delay_line[i]; }

linear_dc_blocker::linear_dc_blocker(int d)
        : d(d)
        , moving_averages(d) {}

double linear_dc_blocker::filter(double x) {
    x = moving_averages.filter(x);
    return moving_averages.get_averager().get_index(d - 1) - x;
}

void linear_dc_blocker::clear() { moving_averages.clear(); }

extra_linear_dc_blocker::extra_linear_dc_blocker(int d)
        : d(d)
        , delay_line(d + 2)
        , moving_averages(d) {}

double extra_linear_dc_blocker::filter(double x) {
    x = moving_averages.filter(x);
    delay_line.push(moving_averages.get_averager().get_index(d - 1));
    return delay_line[d - 1] - x;
}

void extra_linear_dc_blocker::clear() {
    delay_line.clear();
    moving_averages.clear();
}

}  // namespace filter
