#pragma once

#include "fftw3.h"

#include "stl_wrappers.h"

#include "glog/logging.h"

#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>

/// This namespace houses all of the machinery for multiband crossover
/// filtering.
namespace filter {

/// Interface for the most generic boring filter.
class Filter {
public:
    virtual ~Filter() noexcept = default;
    /// Given a vector of data, return a bandpassed version of the data.
    virtual void filter(std::vector<float> &data) = 0;
};

class Lopass : public virtual Filter {
public:
    /// A hipass has mutable cutoff and samplerate.
    virtual void set_params(float co, float s);
    float cutoff, sr;
};

/// Interface for a plain boring hipass filter.
class Hipass : public virtual Filter {
public:
    /// A hipass has mutable cutoff and samplerate.
    virtual void set_params(float co, float s);
    float cutoff, sr;
};

/// Interface for a plain boring bandpass filter.
class Bandpass : public virtual Filter {
public:
    /// A hipass has mutable lopass, hipass, and samplerate.
    virtual void set_params(float l, float h, float s);
    float lo, hi, sr;
};

// In this episode:
// How to work with FFTW in not a dumb way in C++

struct SelfDestructPlan {
    explicit SelfDestructPlan(const fftwf_plan &plan);
    virtual ~SelfDestructPlan() noexcept;
    operator const fftwf_plan &() const;

private:
    fftwf_plan plan;
};

struct fftwf_ptr_destructor {
    template <typename T>
    void operator()(T t) const noexcept {
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
    explicit FastConvolution(int FFT_LENGTH);
    virtual ~FastConvolution() noexcept = default;

    template <typename T, typename U>
    std::vector<float> convolve(const T &a, const U &b) {
        CHECK(a.size() + b.size() - 1 == FFT_LENGTH);
        forward_fft(r2c, a, r2c_i, r2c_o, acplx);
        forward_fft(r2c, b, r2c_i, r2c_o, bcplx);

        memset(c2r_i.get(), 0, CPLX_LENGTH * sizeof(fftwf_complex));
        std::fill(c2r_o.get(), c2r_o.get() + FFT_LENGTH, 0);

        auto x = acplx.get();
        auto y = bcplx.get();
        auto z = c2r_i.get();

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
        proc::copy(data, i.get());
        fftwf_execute(plan);
        memcpy(results.get(), o.get(), CPLX_LENGTH * sizeof(fftwf_complex));
    }

    const int FFT_LENGTH;
    const int CPLX_LENGTH;

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
    explicit LopassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    void filter(std::vector<float> &data) override;
    void set_params(float co, float s) override;

private:
    static const auto KERNEL_LENGTH = 99;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// An interesting windowed-sinc hipass filter.
class HipassWindowedSinc : public Hipass, public FastConvolution {
public:
    explicit HipassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    void filter(std::vector<float> &data) override;
    void set_params(float co, float s) override;

private:
    static const auto KERNEL_LENGTH = 99;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// An interesting windowed-sinc bandpass filter.
class BandpassWindowedSinc : public Bandpass, public FastConvolution {
public:
    explicit BandpassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    void filter(std::vector<float> &data) override;
    void set_params(float l, float h, float s) override;

private:
    static const auto KERNEL_LENGTH = 99;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// A super-simple biquad filter.
class Biquad : public virtual Filter {
public:
    /// Run the filter foward over some data.
    void filter(std::vector<float> &data) override;

    void set_params(double b0, double b1, double b2, double a1, double a2);

private:
    double b0, b1, b2, a1, a2;
};

template <typename T>
class TwopassFilterWrapper : public T {
public:
    void filter(std::vector<float> &data) override {
        T::filter(data);
        proc::reverse(data);
        T::filter(data);
        proc::reverse(data);
    }
};

/// Simple biquad bandpass filter.
class BandpassBiquad : public Bandpass, public Biquad {
public:
    void set_params(float l, float h, float s) override;
};

class LinkwitzRileySingleLopass : public Lopass, public Biquad {
public:
    void set_params(float cutoff, float sr) override;
};

class LinkwitzRileySingleHipass : public Hipass, public Biquad {
public:
    void set_params(float cutoff, float sr) override;
};

using LinkwitzRileyLopass = TwopassFilterWrapper<LinkwitzRileySingleLopass>;
using LinkwitzRileyHipass = TwopassFilterWrapper<LinkwitzRileySingleHipass>;

/// A linkwitz-riley filter is just a linear-phase lopass and hipass
/// coupled together.
class LinkwitzRileyBandpass : public Bandpass {
public:
    void set_params(float l, float h, float s) override;
    void filter(std::vector<float> &data) override;

private:
    TwopassFilterWrapper<LinkwitzRileyLopass> lopass;
    TwopassFilterWrapper<LinkwitzRileyHipass> hipass;
};

class DCBlocker : public Biquad {
public:
    DCBlocker();
    constexpr static auto R = 0.995;
};
}  // namespace filter
