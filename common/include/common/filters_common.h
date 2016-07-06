#pragma once

#include "fast_convolver.h"
#include "stl_wrappers.h"

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

class LopassWindowedSinc : public Lopass, public FastConvolver {
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
class HipassWindowedSinc : public Hipass, public FastConvolver {
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
class BandpassWindowedSinc : public Bandpass, public FastConvolver {
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
    template <typename... Ts>
    TwopassFilterWrapper(Ts &&... ts)
            : T(std::forward<Ts>(ts)...) {
    }

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
