#pragma once

#include "common/fftw/buffer.h"

#include <complex>
#include <functional>
#include <stdexcept>

/// Structured this way so that I can keep all fftw linkage internal.
class fast_filter final {
public:
    /// Instantiating will create two buffers and set up two fft plans.
    /// It will be cheaper to set one of these up once and re-use it, than to
    /// create a new one every time you need to filter something.
    explicit fast_filter(size_t signal_length);

    fast_filter(const fast_filter&) = delete;
    fast_filter& operator=(const fast_filter&) = delete;
    fast_filter(fast_filter&&) = delete;
    fast_filter& operator=(fast_filter&&) = delete;

    ~fast_filter() noexcept;

    /// Filter takes an arbitrary callback to modify the magnitude/phase of
    /// the signal.
    /// The callback is run once on each bin, in an undefined order.
    /// The arguments are the complex value of the bin, and the relative
    /// frequency of that bin.
    using callback =
            std::function<std::complex<float>(std::complex<float>, float)>;

    /// Filters data. It is safe to supply the same range as the input and
    /// output range. Output range should be as big or bigger than input range.
    template <typename In, typename Out>
    void filter(In begin, In end, Out output_it, const callback& callback) {
        const auto dist{std::distance(begin, end)};
        if (dist > rbuf_.size()) {
            throw std::runtime_error(
                    "fast_filter::filter: input signal is too long");
        }

        rbuf_.zero();
        std::copy(begin, end, rbuf_.begin());
        filter_impl(callback);
        std::copy(rbuf_.begin(), rbuf_.begin() + dist, output_it);
    }

private:
    void filter_impl(const callback& callback);

    fftwf::rbuf rbuf_;

    class impl;
    std::unique_ptr<impl> pimpl_;
};

//----------------------------------------------------------------------------//

/// See antoni2010 equations 19 and 20

double lower_band_edge(double centre, double p, double P, size_t l);
double upper_band_edge(double centre, double p, double P, size_t l);

double band_edge_frequency(int band, size_t bands, double lower, double upper);

template <size_t bands>
std::array<double, bands + 1> band_edge_frequencies(double lower, double upper) {
    std::array<double, bands + 1> ret;
    for (auto i{0u}; i != bands + 1; ++i) {
        ret[i] = band_edge_frequency(i, bands, lower, upper);
    }
    return ret;
}

template <size_t bands>
std::array<double, bands + 1> band_centre_frequencies(double lower, double upper) {
    std::array<double, bands + 1> ret;
    for (auto i{0ul}; i != ret.size(); ++i) {
        ret[i] = band_edge_frequency(i * 2 + 1, bands * 2, lower, upper);
    }
    return ret;
}

template <size_t bands>
std::array<double, bands + 1> band_edge_widths(double lower,
                                              double upper,
                                              double overlap) {
    std::array<double, bands + 1> ret;
    for (int i{0}; i != ret.size(); ++i) {
        ret[i] = std::abs(
                (band_edge_frequency(i * 2, bands * 2, lower, upper) -
                 band_edge_frequency(i * 2 + 1, bands * 2, lower, upper)) *
                overlap);
    }
    return ret;
}

/// frequency: normalized frequency, from 0 to 0.5
/// lower: the normalized lower band edge frequency of this band
/// lower_edge_width: half the absolute width of the lower crossover
/// upper: the normalized upper band edge frequency of this band
/// upper_edge_width: half the absolute width of the upper crossover
/// l: the slope (0 is shallow, higher is steeper)
double compute_bandpass_magnitude(double frequency,
                                 double lower,
                                 double lower_edge_width,
                                 double upper,
                                 double upper_edge_width,
                                 size_t l);

double compute_lopass_magnitude(double frequency,
                               double cutoff,
                               double width,
                               size_t l);

double compute_hipass_magnitude(double frequency,
                               double cutoff,
                               double width,
                               size_t l);
