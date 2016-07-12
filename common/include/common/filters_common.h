#pragma once

#include "fast_convolver.h"
#include "stl_wrappers.h"

#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>
#include <algorithm>

/// This namespace houses all of the machinery for multiband crossover
/// filtering.
namespace filter {

class Lopass {
public:
    /// A hipass has mutable cutoff and samplerate.
    virtual void set_params(float co, float s);
    float cutoff, sr;
};

/// Interface for a plain boring hipass filter.
class Hipass {
public:
    /// A hipass has mutable cutoff and samplerate.
    virtual void set_params(float co, float s);
    float cutoff, sr;
};

/// Interface for a plain boring bandpass filter.
class Bandpass {
public:
    /// A hipass has mutable lopass, hipass, and samplerate.
    virtual void set_params(float l, float h, float s);
    float lo, hi, sr;
};

class LopassWindowedSinc : public Lopass {
public:
    explicit LopassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    template<typename It>
    std::vector<float> filter(It begin, It end) {
        return convolver.convolve(
                std::begin(kernel), std::end(kernel), begin, end);
    }

    void set_params(float co, float s) override;

private:
    static const auto KERNEL_LENGTH = 99;
    FastConvolver convolver;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// An interesting windowed-sinc hipass filter.
class HipassWindowedSinc : public Hipass {
public:
    explicit HipassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    template<typename It>
    std::vector<float> filter(It begin, It end) {
        return convolver.convolve(
                std::begin(kernel), std::end(kernel), begin, end);
    }

    void set_params(float co, float s) override;

private:
    static const auto KERNEL_LENGTH = 99;
    FastConvolver convolver;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// An interesting windowed-sinc bandpass filter.
class BandpassWindowedSinc : public Bandpass {
public:
    explicit BandpassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    template<typename It>
    std::vector<float> filter(It begin, It end) {
        return convolver.convolve(
                std::begin(kernel), std::end(kernel), begin, end);
    }

    void set_params(float l, float h, float s) override;

private:
    static const auto KERNEL_LENGTH = 99;
    FastConvolver convolver;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// A super-simple biquad filter.
class Biquad {
public:
    /// Run the filter foward over some data.
    template<typename It>
    std::vector<float> filter(It begin, It end) {
        std::vector<float> ret(begin, end);

        double z1 = 0;
        double z2 = 0;

        std::for_each(
                std::begin(ret), std::end(ret), [this, &z1, &z2](auto &i) {
                    double out = i * b0 + z1;
                    z1 = i * b1 + z2 - a1 * out;
                    z2 = i * b2 - a2 * out;
                    i = out;
                });

        return ret;
    }

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

    template<typename It>
    std::vector<float> filter(It begin, It end) {
        auto t = T::filter(begin, end);
        return T::filter(std::rbegin(t), std::rend(t));
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

    template <typename It>
    std::vector<float> filter(It begin, It end) {
        auto t = lopass.filter(begin, end);
        return hipass.filter(std::begin(t), std::end(t));
    }

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
