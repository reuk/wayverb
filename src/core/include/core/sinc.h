#pragma once

#include "core/dsp_vector_ops.h"

#include "utilities/aligned/vector.h"

namespace wayverb {
namespace core {

/// sinc t = sin (pi . t) / pi . t
template <typename T>
T sinc(T t) {
    if (t == 0) {
        return 1.0;
    }

    T pit = M_PI * t;
    return sin(pit) / pit;
}

/// Generate a convolution kernel for a lowpass sinc filter (NO WINDOWING!).
template <typename T = float>
util::aligned::vector<T> sinc_kernel(double cutoff, int length) {
    if (!(length % 2)) {
        throw std::runtime_error("Length of sinc filter kernel must be odd.");
    }

    util::aligned::vector<T> ret(length);
    for (auto i = 0ul; i != length; ++i) {
        ret[i] = sinc(2 * cutoff * (i - (length - 1) / 2.0));
    }
    return ret;
}

template <typename T, typename U>
void elementwise_multiply(T& a, const U& b) {
    std::transform(begin(a), end(a), begin(b), begin(a), [](auto i, auto j) {
        return i * j;
    });
}

/// Generate a blackman window of a specific length.
template <typename T = float>
util::aligned::vector<T> blackman(int length) {
    //  Holy magic numbers Batman!
    const auto a0 = 7938.0 / 18608.0;
    const auto a1 = 9240.0 / 18608.0;
    const auto a2 = 1430.0 / 18608.0;

    util::aligned::vector<T> ret(length);
    for (auto i = 0ul; i != length; ++i) {
        const auto offset = i / (length - 1.0);
        ret[i] = (a0 - a1 * cos(2 * M_PI * offset) +
                  a2 * cos(4 * M_PI * offset));
    }
    return ret;
}

template <typename T = float>
T hanning_point(double f) {
    return 0.5 - 0.5 * cos(2 * M_PI * f);
}

/// Generate the right half of a hanning window
template <typename T = float>
util::aligned::vector<T> right_hanning(int length) {
    util::aligned::vector<T> ret(length);
    for (auto i = 0; i != length; ++i) {
        ret[i] = hanning_point(0.5 + (i / (2 * (length - 1.0))));
    }
    return ret;
}

template <typename T = float>
util::aligned::vector<T> windowed_sinc_kernel(double cutoff, int length) {
    const auto window = blackman<T>(length);
    auto kernel = sinc_kernel<T>(cutoff, length);
    elementwise_multiply(kernel, window);
    return kernel;
}

/// Generate a windowed, normalized low-pass sinc filter kernel of a specific
/// length.
template <typename T = float>
util::aligned::vector<T> lopass_sinc_kernel(double sr,
                                            double cutoff,
                                            int length) {
    const auto kernel = windowed_sinc_kernel<T>(cutoff / sr, length);
    //    kernel_normalize(kernel);
    return kernel;
}

/// Generate a windowed, normalized high-pass sinc filter kernel of a specific
/// length.
template <typename T = float>
util::aligned::vector<T> hipass_sinc_kernel(double sr,
                                            double cutoff,
                                            int length) {
    auto kernel = windowed_sinc_kernel<T>(cutoff / sr, length);
    std::for_each(begin(kernel), end(kernel), [](auto& i) { i *= -1; });
    kernel[(length - 1) / 2] += 1;
    //    kernel_normalize(kernel);
    return kernel;
}

template <typename T = float>
util::aligned::vector<T> bandpass_sinc_kernel(double sr,
                                              double lo,
                                              double hi,
                                              int length) {
    const auto lop = lopass_sinc_kernel(sr, lo, length);
    auto kernel = lopass_sinc_kernel(sr, hi, length);
    std::transform(begin(kernel),
                   end(kernel),
                   lop.begin(),
                   kernel.begin(),
                   [](auto a, auto b) { return a - b; });
    //    kernel_normalize(kernel);
    return kernel;
}

}  // namespace core
}  // namespace wayverb
