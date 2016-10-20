#pragma once

#include "frequency_domain/buffer.h"

#include <vector>

namespace frequency_domain {

/// This is structured like this so that I don't have to make the fftwf header
/// public...
class convolver final {
public:
    explicit convolver(size_t fft_length);

    convolver(const convolver&) = delete;
    convolver& operator=(const convolver&) = delete;
    convolver(convolver&&) = delete;
    convolver& operator=(convolver&&) = delete;

    ~convolver() noexcept;

    template <typename T, typename U>
    auto convolve(const T& a, const U& b) {
        using std::begin;
        using std::end;
        return convolve(begin(a), end(a), begin(b), end(b));
    }

    template <typename It, typename Jt>
    auto convolve(It a_begin, It a_end, Jt b_begin, Jt b_end) {
        const auto d0 = std::distance(a_begin, a_end);
        const auto d1 = std::distance(b_begin, b_end);
        if (d0 + d1 - 1 != get_fft_length()) {
            throw std::runtime_error{"inputs must sum to FFT_LENGTH + 1"};
        }

        r2c_i_.zero();
        std::copy(a_begin, a_end, r2c_i_.begin());
        forward_fft_a();

        r2c_i_.zero();
        std::copy(b_begin, b_end, r2c_i_.begin());
        forward_fft_b();

        return convolve_impl();
    }

    size_t get_fft_length() const;

private:
    void forward_fft_a();
    void forward_fft_b();
    std::vector<float> convolve_impl();

    rbuf r2c_i_;

    class impl;
    std::unique_ptr<impl> pimpl_;
};

}  // namespace frequency_domain
