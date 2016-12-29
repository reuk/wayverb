#pragma once

#include "utilities/aligned/vector.h"

#include <cmath>
#include <stdexcept>
#include <vector>

namespace wayverb {
namespace core {
namespace kernels {

double gaussian(double t, double o);
double sin_modulated_gaussian(double t, double o);
double gaussian_dash(double t, double o);

template <typename T, typename Func>
util::aligned::vector<T> gauss_like_kernel(double fc, const Func& func) {
    const auto o = 1.0 / (2.0 * M_PI * fc);
    const ptrdiff_t delay = std::ceil(8.0 * o);
    const auto l = delay * 2 + 1;
    util::aligned::vector<T> ret;
    ret.reserve(l);
    for (ptrdiff_t i = 0; i != l; ++i) {
        ret.emplace_back(func(i - delay, o));
    }
    return ret;
}

template <typename T = float>
auto gen_gaussian(double fc) {
    return gauss_like_kernel<T>(fc, gaussian);
}

template <typename T = float>
auto gen_sin_modulated_gaussian(double fc) {
    return gauss_like_kernel<T>(fc, sin_modulated_gaussian);
}

template <typename T = float>
auto gen_gaussian_dash(double fc) {
    return gauss_like_kernel<T>(fc, gaussian_dash);
}

/// t = time
/// f = peak frequency
double ricker(double t, double f);

template <typename T = float>
util::aligned::vector<T> gen_ricker(double fc) {
    const ptrdiff_t delay = std::ceil(1.0 / fc);
    const auto l = delay * 2 + 1;
    util::aligned::vector<T> ret;
    ret.reserve(l);
    for (ptrdiff_t i = 0; i != l; ++i) {
        ret.emplace_back(ricker(i - delay, fc));
    }
    return ret;
}

}  // namespace kernels
}  // namespace core
}  // namespace wayverb
