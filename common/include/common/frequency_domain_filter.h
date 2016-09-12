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
        if (dist != rbuf_.size()) {
            throw std::runtime_error(
                    "filter input have size equal to that used to initialise "
                    "the filter");
        }

        std::copy(begin, end, rbuf_.begin());
        filter_impl(callback);
        std::copy(rbuf_.begin(), rbuf_.end(), output_it);
    }

private:
    void filter_impl(const callback& callback);

    fftwf::rbuf rbuf_;

    class impl;
    std::unique_ptr<impl> pimpl_;
};

//----------------------------------------------------------------------------//

/// See antoni2010 equations 19 and 20

float lower_band_edge(float centre, float p, float P, size_t l);
float upper_band_edge(float centre, float p, float P, size_t l);
