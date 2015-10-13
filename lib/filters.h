#pragma once

#include "logger.h"

#include "fftw3.h"

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <memory>

template <typename T>
auto max_mag(const T & t) {
    return std::accumulate(t.begin(),
                           t.end(),
                           typename T::value_type(0),
                           [](auto a, auto b) { return std::max(a, std::fabs(b)); });
}

template <typename T>
void normalize(T & t) {
    auto mag = max_mag(t);
    if (mag != 0) {
        std::transform(
            t.begin(), t.end(), t.begin(), [mag](auto i) { return i / mag; });
    }
}

/// sinc t = sin (pi . t) / pi . t
template <typename T>
T sinc(const T & t) {
    T pit = M_PI * t;
    return sin(pit) / pit;
}

/// Generate a convolution kernel for a lowpass sinc filter (NO WINDOWING!).
template <typename T = float>
std::vector<T> sinc_kernel(double cutoff, unsigned long length) {
    if (!(length % 2))
        throw std::runtime_error("Length of sinc filter kernel must be odd.");

    std::vector<T> ret(length);
    for (auto i = 0u; i != length; ++i) {
        if (i == ((length - 1) / 2))
            ret[i] = 1;
        else
            ret[i] = sinc(2 * cutoff * (i - (length - 1) / 2.0));
    }
    return ret;
}

/// Generate a blackman window of a specific length.
template <typename T = float>
std::vector<T> blackman(unsigned long length) {
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

template<typename T = float>
std::vector<T> windowed_sinc_kernel(double cutoff, unsigned long length) {
    auto window = blackman<T>(length);
    auto kernel = sinc_kernel<T>(cutoff, length);
    std::transform(begin(window),
              end(window),
              begin(kernel),
              begin(kernel),
              [](auto i, auto j) { return i * j; });
    normalize(kernel);
    return kernel;
}

/// Generate a windowed, normalized low-pass sinc filter kernel of a specific
/// length.
template <typename T = float>
std::vector<T> lopass_kernel(double sr, double cutoff, unsigned long length) {
    return windowed_sinc_kernel(cutoff / sr, length);
}

// In this episode:
// How to work with FFTW in not a dumb way in C++

struct SelfDestructPlan {
    SelfDestructPlan(const fftwf_plan & plan);
    virtual ~SelfDestructPlan() noexcept;
    operator const fftwf_plan & () const;
private:
    fftwf_plan plan;
};

struct fftwf_ptr_destructor {
    template<typename T>
    inline void operator()(T t) const noexcept { fftwf_free(t); }
};

using fftwf_r = std::unique_ptr<float, fftwf_ptr_destructor>;
using fftwf_c = std::unique_ptr<fftwf_complex, fftwf_ptr_destructor>;

class FastConvolution {
public:
    /// An fftconvolover has a constant length.
    /// This means you can reuse it for lots of different convolutions
    /// without reallocating memory, as long as they're all the same size.
    FastConvolution (unsigned long FFT_LENGTH);
    virtual ~FastConvolution() noexcept = default;

    std::vector<float> convolve(const std::vector<float> & a, const std::vector<float> & b);
private:
    void forward_fft
    (   const SelfDestructPlan & plan
    ,   const std::vector<float> & data
    ,   const fftwf_r & i
    ,   const fftwf_c & o
    ,   const fftwf_c & results
    );

    const unsigned long FFT_LENGTH;
    const unsigned long CPLX_LENGTH;

    fftwf_r r2c_i;
    fftwf_c r2c_o;
    fftwf_c c2r_i;
    fftwf_r c2r_o;
    fftwf_c acplx;
    fftwf_c bcplx;

    SelfDestructPlan r2c;
    SelfDestructPlan c2r;
};
