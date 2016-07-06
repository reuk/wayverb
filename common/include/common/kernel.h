#pragma once

#include <cmath>
#include <stdexcept>
#include <vector>

namespace kernels {

inline double gaussian(double t, double bandwidth) {
    return std::pow(M_E, -std::pow(t, 2.0) / std::pow(bandwidth, 2.0);
}

inline double sin_modulated_gaussian(double t,
                                     double bandwidth,
                                     double frequency) {
    return -gaussian(t, bandwidth) * sin(2.0 * M_PI * frequency * t);
}

template <typename T = float>
std::vector<T> sin_modulated_gaussian_kernel(double sampling_frequency) {
    const auto upper = sampling_frequency / 4;
    const auto o = 2 / (M_PI * upper);
    const auto l = std::ceil(8 * o * sampling_frequency) * 2;
    const auto time_offset = l / (2 * sampling_frequency);
    std::vector<T> ret(l);
    for (auto i = 0u; i != l; ++i) {
        ret[i] = sin_modulated_gaussian(
                (i / sampling_frequency) - time_offset, o, upper * 0.5);
    }
    return ret;
}

/// t = time
/// f = peak frequency
inline double ricker(double t, double f) {
    //  see http://docs.scipy.org/doc/scipy/reference/generated/scipy.signal.ricker.html
    //  and http://wiki.seg.org/wiki/Dictionary:Ricker_wavelet
    const auto u = M_PI * M_PI * f * f * t * t;
    return (1.0 - 2.0 * u) * std::exp(-u);
}

template<typename T = float>
std::vector<T> ricker_kernel(double sampling_frequency) {
    //  TODO find optimum kernel length
    const auto upper = sampling_frequency / 4;
    const auto o = 2 / (M_PI * upper);
    const auto l = std::ceil(8 * o * sampling_frequency) * 2;
    const auto time_offset = l / (2 * sampling_frequency);
    std::vector<T> ret(l);
    for (auto i = 0u; i != l; ++i) {
        ret[i] = ricker((i / sampling_frequency) - time_offset, upper * 0.5);
    }
    return ret;
}



}  // namespace kernels
