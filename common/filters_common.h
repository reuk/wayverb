#pragma once

#include "fftw3.h"

#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <memory>
#include <cstring>

/// This namespace houses all of the machinery for multiband crossover
/// filtering.

/// Interface for the most generic boring filter.
class Filter {
public:
    /// Given a vector of data, return a bandpassed version of the data.
    virtual void filter(std::vector<float> &data) = 0;
};

class Lopass : public Filter {
public:
    virtual ~Lopass() noexcept = default;
    /// A hipass has mutable cutoff and samplerate.
    virtual void setParams(float co, float s);
    float cutoff, sr;
};

/// Interface for a plain boring hipass filter.
class Hipass : public Filter {
public:
    virtual ~Hipass() noexcept = default;
    /// A hipass has mutable cutoff and samplerate.
    virtual void setParams(float co, float s);
    float cutoff, sr;
};

/// Interface for a plain boring bandpass filter.
class Bandpass : public Filter {
public:
    virtual ~Bandpass() noexcept = default;
    /// A hipass has mutable lopass, hipass, and samplerate.
    virtual void setParams(float l, float h, float s);
    float lo, hi, sr;
};

// In this episode:
// How to work with FFTW in not a dumb way in C++

struct SelfDestructPlan {
    SelfDestructPlan(const fftwf_plan &plan);
    virtual ~SelfDestructPlan() noexcept;
    operator const fftwf_plan &() const;

private:
    fftwf_plan plan;
};

struct fftwf_ptr_destructor {
    template <typename T>
    inline void operator()(T t) const noexcept {
        fftwf_free(t);
    }
};

using fftwf_r = std::unique_ptr<float, fftwf_ptr_destructor>;
using fftwf_c = std::unique_ptr<fftwf_complex, fftwf_ptr_destructor>;

class FastConvolution {
public:
    /// An fftconvolover has a constant length.
    /// This means you can reuse it for lots of different convolutions
    /// without reallocating memory, as long as they're all the same size.
    FastConvolution(unsigned long FFT_LENGTH);
    virtual ~FastConvolution() noexcept = default;

    template <typename T, typename U>
    std::vector<float> convolve(const T &a, const U &b) {
        forward_fft(r2c, a, r2c_i, r2c_o, acplx);
        forward_fft(r2c, b, r2c_i, r2c_o, bcplx);

        memset(c2r_i.get(), 0, CPLX_LENGTH * sizeof(fftwf_complex));
        std::fill(c2r_o.get(), c2r_o.get() + FFT_LENGTH, 0);

        auto x = acplx.get();
        auto y = bcplx.get();
        auto z = c2r_i.get();

        // simd is for better people
        for (; z != c2r_i.get() + CPLX_LENGTH; ++x, ++y, ++z) {
            (*z)[0] += (*x)[0] * (*y)[0] - (*x)[1] * (*y)[1];
            (*z)[1] += (*x)[0] * (*y)[1] + (*x)[1] * (*y)[0];
        }

        fftwf_execute(c2r);

        return std::vector<float>(c2r_o.get(), c2r_o.get() + FFT_LENGTH);
    }

private:
    template <typename T>
    void forward_fft(const SelfDestructPlan &plan,
                     const T &data,
                     const fftwf_r &i,
                     const fftwf_c &o,
                     const fftwf_c &results) {
        std::fill(i.get(), i.get() + FFT_LENGTH, 0);
        std::copy(data.begin(), data.end(), i.get());
        fftwf_execute(plan);
        memcpy(results.get(), o.get(), CPLX_LENGTH * sizeof(fftwf_complex));
    }

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

class LopassWindowedSinc : public Lopass, public FastConvolution {
public:
    LopassWindowedSinc(unsigned long inputLength);
    virtual ~LopassWindowedSinc() noexcept = default;

    /// Filter a vector of data.
    virtual void filter(std::vector<float> &data);
    virtual void setParams(float co, float s);

private:
    static const auto KERNEL_LENGTH = 29;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// An interesting windowed-sinc hipass filter.
class HipassWindowedSinc : public Hipass, public FastConvolution {
public:
    HipassWindowedSinc(unsigned long inputLength);
    virtual ~HipassWindowedSinc() noexcept = default;

    /// Filter a vector of data.
    virtual void filter(std::vector<float> &data);
    virtual void setParams(float co, float s);

private:
    static const auto KERNEL_LENGTH = 29;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// An interesting windowed-sinc bandpass filter.
class BandpassWindowedSinc : public Bandpass, public FastConvolution {
public:
    BandpassWindowedSinc(unsigned long inputLength);
    virtual ~BandpassWindowedSinc() noexcept = default;

    /// Filter a vector of data.
    virtual void filter(std::vector<float> &data);
    virtual void setParams(float l, float h, float s);

private:
    static const auto KERNEL_LENGTH = 29;

    /// Fetch a convolution kernel for a bandpass filter with the given
    /// paramters.
    static std::vector<float> bandpassKernel(float sr, float lo, float hi);

    std::array<float, KERNEL_LENGTH> kernel;
};

/// A super-simple biquad filter.
class Biquad {
public:
    virtual ~Biquad() noexcept = default;
    /// Run the filter foward over some data.
    void onepass(std::vector<float> &data);

    /// Run the filter forward then backward over some data.
    void twopass(std::vector<float> &data);

    void setParams(double b0, double b1, double b2, double a1, double a2);

private:
    double b0, b1, b2, a1, a2;
};

/// Simple biquad bandpass filter.
class OnepassBandpassBiquad : public Bandpass, public Biquad {
public:
    virtual ~OnepassBandpassBiquad() noexcept = default;
    void setParams(float l, float h, float s);
    void filter(std::vector<float> &data);
};

/// Simple biquad bandpass filter.
class TwopassBandpassBiquad : public OnepassBandpassBiquad {
public:
    virtual ~TwopassBandpassBiquad() noexcept = default;
    void filter(std::vector<float> &data);
};

/// A linkwitz-riley filter is just a linear-phase lopass and hipass
/// coupled together.
class LinkwitzRiley : public Bandpass {
public:
    virtual ~LinkwitzRiley() noexcept = default;
    void setParams(float l, float h, float s);
    void filter(std::vector<float> &data);

private:
    Biquad lopass, hipass;
};
