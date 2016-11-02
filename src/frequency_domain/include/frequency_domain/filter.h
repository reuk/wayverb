#pragma once

#include "frequency_domain/buffer.h"

#include <complex>
#include <functional>
#include <stdexcept>

namespace frequency_domain {

/// Structured this way so that I can keep all fftw linkage internal.
class filter final {
public:
    /// Instantiating will create two buffers and set up two fft plans.
    /// It will be cheaper to set one of these up once and re-use it, than to
    /// create a new one every time you need to filter something.
    explicit filter(size_t signal_length);

    filter(const filter&) = delete;
    filter& operator=(const filter&) = delete;
    filter(filter&&) = delete;
    filter& operator=(filter&&) = delete;

    ~filter() noexcept;

    /// Filter takes an arbitrary callback to modify the magnitude/phase of
    /// the signal.
    /// The callback is run once on each bin, in an undefined order.
    /// The arguments are the complex value of the bin, and the relative
    /// frequency of that bin.
    using callback =
            std::function<std::complex<float>(std::complex<float>, float)>;

    /// Filters data. It is safe to supply the same range as the input and
    /// output range. Output range should be as big or bigger than input range.
    /// TODO this is going to be slow because it will do a whole bunch of 
    /// virtual call to the callback parameter.
    /// Not sure how to speed this up while keeping all the fft stuff behind a
    /// compiler firewall.
    template <typename In, typename Out>
    void run(In begin, In end, Out output_it, const callback& callback) {
        const auto dist = std::distance(begin, end);
        if (dist > rbuf_.size()) {
            throw std::runtime_error(
                    "filter::filter: input signal is too long");
        }

        rbuf_.zero();
        std::copy(begin, end, rbuf_.begin());
        filter_impl(callback);
        std::copy(rbuf_.begin(), rbuf_.begin() + dist, output_it);
    }

private:
    void filter_impl(const callback& callback);

    rbuf rbuf_;

    class impl;
    std::unique_ptr<impl> pimpl_;
};

}  // namespace frequency_domain
