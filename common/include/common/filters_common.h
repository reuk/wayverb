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

///	FIR filter concept:
///		* implements a method 'filter' which takes two iterators and returns
///		  a filtered vector of float.

class lopass_windowed_sinc final {
public:
    explicit lopass_windowed_sinc(int inputLength);

    /// Filter a vector of data.
    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
        return convolver_.convolve(
                std::begin(kernel_), std::end(kernel_), begin, end);
    }

    void set_params(double co, double s);

private:
    static constexpr size_t KERNEL_LENGTH{99};
    fast_convolver convolver_;
    std::array<float, KERNEL_LENGTH> kernel_;
};

/// An interesting windowed-sinc hipass filter.
class hipass_windowed_sinc final {
public:
    explicit hipass_windowed_sinc(int inputLength);

    /// Filter a vector of data.
    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
        return convolver_.convolve(
                std::begin(kernel_), std::end(kernel_), begin, end);
    }

    void set_params(double co, double s);

private:
    static constexpr size_t KERNEL_LENGTH{99};
    fast_convolver convolver_;
    std::array<float, KERNEL_LENGTH> kernel_;
};

/// An interesting windowed-sinc bandpass filter.
class bandpass_windowed_sinc final {
public:
    explicit bandpass_windowed_sinc(int inputLength);

    /// Filter a vector of data.
    template <typename It>
    aligned::vector<float> filter(It begin, It end) {
        return convolver_.convolve(
                std::begin(kernel_), std::end(kernel_), begin, end);
    }

    void set_params(double l, double h, double s);

private:
    static constexpr size_t KERNEL_LENGTH{99};
    fast_convolver convolver_;
    std::array<float, KERNEL_LENGTH> kernel_;
};

//----------------------------------------------------------------------------//

///	IIR filter concept:
///		* implements a method 'filter' which takes a single double, filters it,
///		  and returns a single filtered double.
///		* implements a method 'clear' which zeros any state that might affect
///		  future output from 'filter' but which does not change the
///		  characteristics of the filter.

/// A super-simple biquad filter.
///	Adheres to the IIR filter concept.

class biquad final {
public:
    struct coefficients final {
        double b0{0}, b1{0}, b2{0}, a1{0}, a2{0};
    };

    constexpr biquad(const coefficients& coefficients)
            : coefficients_{coefficients} {}

    ///	Run the filter for one step.
    constexpr double filter(double i) {
        const auto out{i * coefficients_.b0 + z1_};
        z1_ = i * coefficients_.b1 - coefficients_.a1 * out + z2_;
        z2_ = i * coefficients_.b2 - coefficients_.a2 * out;
        return out;
    }

    ///	Resets delay lines, does *not* affect filter coefficients.
    constexpr void clear() { z1_ = z2_ = 0; }

private:
    coefficients coefficients_;
    double z1_{0}, z2_{0};
};

//----------------------------------------------------------------------------//

biquad::coefficients compute_bandpass_biquad_coefficients(double lo,
                                                          double hi,
                                                          double sr);
biquad::coefficients compute_linkwitz_riley_lopass_coefficients(double cutoff,
                                                                double sr);
biquad::coefficients compute_linkwitz_riley_hipass_coefficients(double cutoff,
                                                                double sr);
biquad::coefficients compute_dc_blocker_coefficients_();

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
    biquad lopass{compute_linkwitz_riley_lopass_coefficients(hi, s)};
    run_two_pass(lopass, begin, end);
    biquad hipass{compute_linkwitz_riley_hipass_coefficients(lo, s)};
    run_two_pass(hipass, begin, end);
}

//----------------------------------------------------------------------------//

template <size_t... i>
constexpr auto make_biquads(
        const std::array<biquad::coefficients, sizeof...(i)>& coefficients,
        std::index_sequence<i...>) {
    return std::array<biquad, sizeof...(i)>{{biquad{coefficients[i]}...}};
}

template <size_t num>
constexpr auto make_biquads(
        const std::array<biquad::coefficients, num>& coefficients) {
    return make_biquads(coefficients, std::make_index_sequence<num>{});
}

///	Adheres to the IIR filter concept.
template <size_t num>
class series_biquads final {
public:
    static constexpr auto num_biquads{num};
    static constexpr auto order{num_biquads * 2};

    constexpr series_biquads(
            const std::array<biquad::coefficients, num_biquads>& coefficients)
            : biquads_{make_biquads(coefficients)} {}

    constexpr double filter(double i) {
        for (auto& biquad : biquads_) {
            i = biquad.filter(i);
        }
        return i;
    }

    constexpr void clear() {
        for (auto& biquad : biquads_) {
            biquad.clear();
        }
    }

private:
    std::array<biquad, num_biquads> biquads_;
};

template <size_t num>
constexpr auto make_series_biquads(
        const std::array<biquad::coefficients, num>& coefficients) {
    return series_biquads<num>{coefficients};
}

//----------------------------------------------------------------------------//

biquad::coefficients compute_lopass_butterworth_segment(double cf,
                                                        size_t order,
                                                        size_t segment);

biquad::coefficients compute_hipass_butterworth_segment(double cf,
                                                        size_t order,
                                                        size_t segment);

template <typename Callback, size_t... i>
auto compute_butterworth_segments(double cf,
                                  Callback callback,
                                  std::index_sequence<i...>) {
    const auto order{sizeof...(i) * 2};
    return std::array<biquad::coefficients, sizeof...(i)>{
            {callback(cf, order, i)...}};
}

template <size_t num, typename Callback>
auto compute_butterworth_coefficients(
    double cutoff, double sr, Callback callback) {
    return compute_butterworth_segments(std::tan(M_PI * cutoff / sr),
                                        callback,
                                        std::make_index_sequence<num>{});
}

template <size_t num>
auto compute_hipass_butterworth_coefficients(
    double cutoff, double sr) {
    return compute_butterworth_coefficients<num>(
            cutoff, sr, compute_hipass_butterworth_segment);
}

template <size_t num>
auto compute_lopass_butterworth_coefficients(
    double cutoff, double sr) {
    return compute_butterworth_coefficients<num>(
            cutoff, sr, compute_lopass_butterworth_segment);
}

template <typename IIR>
aligned::vector<float> impulse_response(IIR& filt, size_t steps) {
    aligned::vector<float> ret(steps, 0.0f);
    ret.front() = 1;
    run_one_pass(filt, ret.begin(), ret.end());
    return ret;
}

}  // namespace filter
