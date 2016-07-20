#pragma once

#include "fftwf_helpers.h"
#include "stl_wrappers.h"

#include "common/aligned/vector.h"

#include "glog/logging.h"

#include <vector>

/// A pretty speedy fixed-size offline convolver.
class FastConvolver {
public:
    /// An fftconvolover has a constant length.
    /// This means you can reuse it for lots of different convolutions
    /// without reallocating memory, as long as they're all the same size.
    /// FFT_LENGTH should be equal to the sum of lengths - 1 of the
    /// signals to be convolved.
    explicit FastConvolver(size_t FFT_LENGTH);

    FastConvolver(const FastConvolver &) = default;
    FastConvolver &operator=(const FastConvolver &) = default;
    FastConvolver(FastConvolver &&) noexcept        = default;
    FastConvolver &operator=(FastConvolver &&) noexcept = default;
    virtual ~FastConvolver() noexcept                   = default;

    /// Collect the gems Bentley Bear.
    template <typename T, typename U>
    aligned::vector<float> convolve(const T &a, const U &b) {
        return convolve(std::begin(a), std::end(a), std::begin(b), std::end(b));
    }

    template <typename It, typename Jt>
    aligned::vector<float> convolve(It a_begin,
                                    It a_end,
                                    Jt b_begin,
                                    Jt b_end) {
        const auto d0 = std::distance(a_begin, a_end);
        const auto d1 = std::distance(b_begin, b_end);
        CHECK(d0 + d1 - 1 == FFT_LENGTH);
        forward_fft(r2c, a_begin, a_end, r2c_i, r2c_o, acplx);
        forward_fft(r2c, b_begin, b_end, r2c_i, r2c_o, bcplx);

        c2r_i.zero();
        c2r_o.zero();

        auto x = acplx.begin();
        auto y = bcplx.begin();
        auto z = c2r_i.begin();

        for (; z != c2r_i.end(); ++x, ++y, ++z) {
            (*z)[0] += (*x)[0] * (*y)[0] - (*x)[1] * (*y)[1];
            (*z)[1] += (*x)[0] * (*y)[1] + (*x)[1] * (*y)[0];
        }

        fftwf_execute(c2r);

        aligned::vector<float> ret(c2r_o.begin(), c2r_o.end());

        for (auto &i : ret) {
            i /= FFT_LENGTH;
        }

        return ret;
    }

private:
    template <typename It>
    void forward_fft(const FftwfPlan &plan,
                     It begin,
                     It end,
                     fftwf_r &i,
                     fftwf_c &o,
                     fftwf_c &results) {
        i.zero();
        std::copy(begin, end, i.begin());
        fftwf_execute(plan);
        results = o;
    }

    const size_t FFT_LENGTH;
    const size_t CPLX_LENGTH;

    fftwf_r r2c_i;
    fftwf_c r2c_o;
    fftwf_c c2r_i;
    fftwf_r c2r_o;
    fftwf_c acplx;
    fftwf_c bcplx;

    FftwfPlan r2c;
    FftwfPlan c2r;
};
