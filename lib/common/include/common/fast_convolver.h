#pragma once

#include "common/aligned/vector.h"
#include "common/fftw/buffer.h"
#include "common/stl_wrappers.h"

/// This is structured like this so that I don't have to make the fftwf header
/// public...
class fast_convolver final {
public:
    explicit fast_convolver(size_t fft_length);

    fast_convolver(const fast_convolver&) = delete;
    fast_convolver& operator=(const fast_convolver&) = delete;
    fast_convolver(fast_convolver&&) = delete;
    fast_convolver& operator=(fast_convolver&&) = delete;

	~fast_convolver() noexcept;

    template <typename T, typename U>
    aligned::vector<float> convolve(const T &a, const U &b) {
        using std::begin;
        using std::end;
        return convolve(begin(a), end(a), begin(b), end(b));
    }

    template <typename It, typename Jt>
    aligned::vector<float> convolve(It a_begin,
                                    It a_end,
                                    Jt b_begin,
                                    Jt b_end) {
        const auto d0{std::distance(a_begin, a_end)};
        const auto d1{std::distance(b_begin, b_end)};
        if (d0 + d1 - 1 != get_fft_length()) {
            throw std::runtime_error("inputs must sum to FFT_LENGTH + 1");
        }

		r2c_i.zero();
        std::copy(a_begin, a_end, r2c_i.begin());
        forward_fft_a();

		r2c_i.zero();
        std::copy(b_begin, b_end, r2c_i.begin());
        forward_fft_b();

        return convolve_impl();
    }

    size_t get_fft_length() const;

private:
    void forward_fft_a();
    void forward_fft_b();
    aligned::vector<float> convolve_impl();

    fftwf::rbuf r2c_i;

    class impl;
    std::unique_ptr<impl> pimpl;
};
