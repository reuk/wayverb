#pragma once

#include "fast_convolver.h"
#include "stl_wrappers.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <memory>
#include <vector>

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
    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
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
    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
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
    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
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
    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
        aligned::vector<float> ret(begin, end);

        double z1 = 0;
        double z2 = 0;

        std::for_each(
                std::begin(ret), std::end(ret), [this, &z1, &z2](auto& i) {
                    double out = i * b0 + z1;
                    z1         = i * b1 + z2 - a1 * out;
                    z2         = i * b2 - a2 * out;
                    i          = out;
                });

        return ret;
    }

    void clear() {}

    void set_params(double b0, double b1, double b2, double a1, double a2);

private:
    double b0, b1, b2, a1, a2;
};

template <typename Filter, typename It>
auto run_one_pass(Filter& filter, It begin, It end) {
    filter.clear();
    return filter.filter(begin, end);
}

template <typename Filter, typename It>
auto run_two_pass(Filter& filter, It begin, It end) {
    auto t = run_one_pass(filter, begin, end);
    auto u = run_one_pass(filter, std::crbegin(t), std::crend(t));
    std::reverse(u.begin(), u.end());
    return u;
}

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

/// A linkwitz-riley filter is just a linear-phase lopass and hipass
/// coupled together.
class LinkwitzRileyBandpass : public Bandpass {
public:
    void set_params(float l, float h, float s) override;

    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
        auto t = run_two_pass(lopass, begin, end);
        return run_two_pass(hipass, t.begin(), t.end());
    }

    void clear() {
        lopass.clear();
        hipass.clear();
    }

private:
    LinkwitzRileySingleLopass lopass;
    LinkwitzRileySingleHipass hipass;
};

class DCBlocker : public Biquad {
public:
    DCBlocker();
    constexpr static auto R = 0.995;
};
}  // namespace filter
