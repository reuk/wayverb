#pragma once

#include <cmath>
#include <stdexcept>
#include <vector>

double gaussian(double t, double t0, double bandwidth) {
    return std::pow(M_E, -std::pow(t - t0, 2) / (std::pow(bandwidth, 2)));
}

double sin_modulated_gaussian(double t,
                              double t0,
                              double bandwidth,
                              double frequency) {
    return -gaussian(t, t0, bandwidth) * sin(frequency * (t - t0));
}

template <typename T = float>
std::vector<T> waveguide_kernel(double sampling_frequency) {
    auto upper = sampling_frequency / 4;
    auto o = 2 / (M_PI * upper);
    auto time_offset = 8 * o;
    auto l = std::ceil(time_offset * sampling_frequency * 2);
    time_offset = (l - 1) / (2 * sampling_frequency);
    std::vector<T> ret(l);
    for (auto i = 0; i != l; ++i) {
        ret[i] = sin_modulated_gaussian(i / sampling_frequency,
                                        time_offset,
                                        o,
                                        upper / 2);
    }
//    normalize(ret);
    return ret;
}
