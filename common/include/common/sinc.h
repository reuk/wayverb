#pragma once

#include "stl_wrappers.h"

#include <numeric>
#include <vector>

template <typename T>
inline float sum(const T& t) {
    return proc::accumulate(
        t, 0.0f, [](const auto& a, const auto& b) { return a + sum(b); });
}

template <>
inline float sum(const float& t) {
    return t;
}

template <typename T>
inline auto count(const T& t) {
    return proc::accumulate(
        t, 0u, [](const auto& a, const auto& b) { return a + count(b); });
}

template <typename T>
inline auto count(const std::vector<T>& coll) {
    return coll.size();
}

template <typename T>
inline auto mean(const T& t) {
    return sum(t) / count(t);
}

template <typename T>
inline float max_mag(const T& t) {
    return proc::accumulate(
        t, 0.0f, [](auto a, auto b) { return std::max(a, max_mag(b)); });
}

template <>
inline float max_mag(const float& t) {
    return std::fabs(t);
}

/// Recursively divide by reference.
template <typename T>
inline void div(T& ret, float f) {
    for (auto& i : ret) {
        div(i, f);
    }
}

/// The base case of the div recursion.
template <>
inline void div(float& ret, float f) {
    ret /= f;
}

/// Recursively multiply by reference.
template <typename T>
inline void mul(T& ret, float f) {
    for (auto& i : ret) {
        mul(i, f);
    }
}

/// The base case of the mul recursion.
template <>
inline void mul(float& ret, float f) {
    ret *= f;
}

/// Find the largest absolute value in an arbitarily nested vector, then
/// divide every item in the vector by that value.
template <typename T>
inline void normalize(T& ret) {
    auto mag = max_mag(ret);
    if (mag != 0) {
        mul(ret, 1.0 / mag);
    }
}

template <typename T>
inline void kernel_normalize(T& ret) {
    auto get_sum = [&ret] { return proc::accumulate(ret, 0.0); };
    auto sum = get_sum();
    if (sum != 0) {
        mul(ret, 1.0 / std::abs(sum));
    }
}

/// sinc t = sin (pi . t) / pi . t
template <typename T>
T sinc(T t) {
    T pit = M_PI * t;
    return sin(pit) / pit;
}

/// Generate a convolution kernel for a lowpass sinc filter (NO WINDOWING!).
//  TODO change centre value - 2 * cutoff?
template <typename T = float>
std::vector<T> sinc_kernel(double cutoff, int length) {
    if (!(length % 2)) {
        throw std::runtime_error("Length of sinc filter kernel must be odd.");
    }

    std::vector<T> ret(length);
    for (auto i = 0u; i != length; ++i) {
        if (i == ((length - 1) / 2)) {
            ret[i] = 1;
            //  ret[i] = 2 * cutoff;
        } else {
            ret[i] = sinc(2 * cutoff * (i - (length - 1) / 2.0));
        }
    }
    return ret;
}

template <typename T, typename U>
void elementwise_multiply(T& a, const U& b) {
    proc::transform(
        a, std::begin(b), std::begin(a), [](auto i, auto j) { return i * j; });
}

/// Generate a blackman window of a specific length.
template <typename T = float>
std::vector<T> blackman(int length) {
    const auto a0 = 7938.0 / 18608.0;
    const auto a1 = 9240.0 / 18608.0;
    const auto a2 = 1430.0 / 18608.0;

    std::vector<T> ret(length);
    for (auto i = 0u; i != length; ++i) {
        const auto offset = i / (length - 1.0);
        ret[i] =
            (a0 - a1 * cos(2 * M_PI * offset) + a2 * cos(4 * M_PI * offset));
    }
    return ret;
}

template <typename T = float>
T hanning_point(double f) {
    return 0.5 - 0.5 * cos(2 * M_PI * f);
}

/// Generate the right half of a hanning window
template <typename T = float>
std::vector<T> right_hanning(int length) {
    std::vector<T> ret(length);
    for (auto i = 0; i != length; ++i) {
        ret[i] = hanning_point(0.5 + (i / (2 * (length - 1.0))));
    }
    return ret;
}

template <typename T = float>
std::vector<T> windowed_sinc_kernel(double cutoff, int length) {
    auto window = blackman<T>(length);
    auto kernel = sinc_kernel<T>(cutoff, length);
    elementwise_multiply(kernel, window);
    return kernel;
}

/// Generate a windowed, normalized low-pass sinc filter kernel of a specific
/// length.
template <typename T = float>
std::vector<T> lopass_sinc_kernel(double sr, double cutoff, int length) {
    auto kernel = windowed_sinc_kernel<T>(cutoff / sr, length);
    kernel_normalize(kernel);
    return kernel;
}

/// Generate a windowed, normalized high-pass sinc filter kernel of a specific
/// length.
template <typename T = float>
std::vector<T> hipass_sinc_kernel(double sr, double cutoff, int length) {
    auto kernel = windowed_sinc_kernel<T>(cutoff / sr, length);
    proc::for_each(kernel, [](auto& i) { i *= -1; });
    kernel[(length - 1) / 2] += 1;
    kernel_normalize(kernel);
    return kernel;
}

template <typename T = float>
std::vector<T> bandpass_sinc_kernel(double sr,
                                    double lo,
                                    double hi,
                                    int length) {
    auto lop = lopass_sinc_kernel(sr, lo, length);
    auto kernel = lopass_sinc_kernel(sr, hi, length);
    proc::transform(kernel, lop.begin(), kernel.begin(), [](auto a, auto b) {
        return a - b;
    });
    kernel_normalize(kernel);
    return kernel;
}
