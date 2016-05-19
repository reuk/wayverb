#pragma once

#include <cmath>
#include <stdexcept>
#include <vector>

inline double gaussian(double t, double t0, double bandwidth) {
    return std::pow(M_E, -std::pow(t - t0, 2) / std::pow(bandwidth, 2));
}

inline double sin_modulated_gaussian(double t,
                                     double t0,
                                     double bandwidth,
                                     double frequency) {
    return -gaussian(t, t0, bandwidth) * sin(2 * M_PI * frequency * (t - t0));
}

template <typename T = float>
std::vector<T> waveguide_kernel(double sampling_frequency) {
    const auto upper = sampling_frequency / 4;
    const auto o = 2 / (M_PI * upper);
    const auto l = std::ceil(8 * o * sampling_frequency) * 2;
    const auto time_offset = l / (2 * sampling_frequency);
    std::vector<T> ret(l);
    for (auto i = 0; i != l; ++i) {
        ret[i] = sin_modulated_gaussian(
            i / sampling_frequency, time_offset, o, upper * 0.5);
    }
    return ret;
}
