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

class LopassWindowedSinc final {
public:
    explicit LopassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    template <typename It>
    aligned::vector<float> filter(It b, It e) {
        return convolver.convolve(kernel.begin(), kernel.end(), b, e);
    }

    void set_params(double co, double s);

private:
    static const auto KERNEL_LENGTH{99};
    fast_convolver convolver;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// An interesting windowed-sinc hipass filter.
class HipassWindowedSinc final {
public:
    explicit HipassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
        return convolver.convolve(
                std::begin(kernel), std::end(kernel), begin, end);
    }

    void set_params(double co, double s);

private:
    static const auto KERNEL_LENGTH{99};
    fast_convolver convolver;
    std::array<float, KERNEL_LENGTH> kernel;
};

/// An interesting windowed-sinc bandpass filter.
class BandpassWindowedSinc final {
public:
    explicit BandpassWindowedSinc(int inputLength);

    /// Filter a vector of data.
    template <typename It>
    aligned::vector<float> filter(It b, It e) {
        return convolver.convolve(kernel.begin(), kernel.end(), b, e);
    }

    void set_params(double l, double h, double s);

private:
    static const auto KERNEL_LENGTH{99};
    fast_convolver convolver;
    std::array<float, KERNEL_LENGTH> kernel;
};

//----------------------------------------------------------------------------//

/// A super-simple biquad filter.
/// Oh hey it's basically a functor
class Biquad final {
public:
    Biquad(double b0, double b1, double b2, double a1, double a2);
    double filter(double i);
    void clear();

private:
    double b0{0}, b1{0}, b2{0}, a1{0}, a2{0};
    double z1{0}, z2{0};
};

//----------------------------------------------------------------------------//

Biquad make_bandpass_biquad(double lo, double hi, double sr);
Biquad make_linkwitz_riley_lopass(double cutoff, double sr);
Biquad make_linkwitz_riley_hipass(double cutoff, double sr);
Biquad make_dc_blocker();

//----------------------------------------------------------------------------//

template <typename Filter, typename It>
void run_one_pass(Filter& filter, It begin, It end) {
    filter.clear();
    for (; begin != end; ++begin) {
        *begin = filter.filter(*begin);
    }
}

template <typename Filter, typename It>
void run_two_pass(Filter& filter, It begin, It end) {
    run_one_pass(filter, begin, end);
    run_one_pass(filter,
                 std::make_reverse_iterator(end),
                 std::make_reverse_iterator(begin));
}

//----------------------------------------------------------------------------//

template <typename It>
void linkwitz_riley_bandpass(double lo, double hi, double s, It begin, It end) {
    auto lopass{make_linkwitz_riley_lopass(hi, s)};
    run_two_pass(lopass, begin, end);
    auto hipass{make_linkwitz_riley_hipass(lo, s)};
    run_two_pass(hipass, begin, end);
}

}  // namespace filter
