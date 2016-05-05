#pragma once

#include <cmath>
#include <stdexcept>
#include <vector>

double gaussian(double t, double bandwidth) {
    return std::pow(M_E, -std::pow(t, 2) / (2 * std::pow(bandwidth, 2)));
}

double sin_modulated_gaussian(double t, double bandwidth, double frequency) {
    return -gaussian(t, bandwidth) * sin(frequency * t);
}

template <typename T = float>
std::vector<T> sin_modulated_gaussian_kernel(int length,
                                             double bandwidth,
                                             double sr) {
    if (length % 2 != 1) {
        throw std::runtime_error("kernel length must be odd");
    }
    std::vector<T> ret(length);
    for (int i = 0; i != length; ++i) {
        ret[i] =
            sin_modulated_gaussian((i - (length / 2)) / sr, bandwidth, sr / 8);
    }
    return ret;
}
