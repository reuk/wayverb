#include "common/frequency_domain_filter.h"
#include "fftw/plan.h"

#include <cmath>

class fast_filter::impl final {
public:
    using rbuf = fftwf::rbuf;
    using cbuf = fftwf::buffer<fftwf_complex>;

    explicit impl(rbuf& rbuf)
            : rbuf_{rbuf}
            , cbuf_{rbuf.size() / 2 + 1}
            , fft_{fftwf_plan_dft_r2c_1d(
                      rbuf.size(), rbuf.data(), cbuf_.data(), FFTW_ESTIMATE)}
            , ifft_{fftwf_plan_dft_c2r_1d(
                      rbuf.size(), cbuf_.data(), rbuf.data(), FFTW_ESTIMATE)} {}

    void filter_impl(const fast_filter::callback& callback) {
        //  Run forward fft, placing fft output into cbuf_.
        fftwf_execute(fft_);

        const auto rbuf_size{rbuf_.size()};
        //  Modify magnitudes in the frequency domain.
        for (auto i{0ul}, end{cbuf_.size()}; i != end; ++i) {
            const auto normalised_frequency{i / static_cast<float>(rbuf_size)};
            auto& bin{cbuf_.data()[i]};
            auto& re{bin[0]};
            auto& im{bin[1]};
            const auto new_value{callback(std::complex<float>{re, im},
                                          normalised_frequency)};
            re = new_value.real();
            im = new_value.imag();
        }

        //  Run inverse fft, placing ifft output back into owner.rbuf_.
        fftwf_execute(ifft_);

        for (auto & i : rbuf_) {
            i /= rbuf_size;
        }
    }

private:
    rbuf& rbuf_;
    cbuf cbuf_;
    fftwf::plan fft_;
    fftwf::plan ifft_;
};

//----------------------------------------------------------------------------//

fast_filter::fast_filter(size_t signal_length)
        : rbuf_{signal_length}
        , pimpl_{std::make_unique<impl>(rbuf_)} {}

fast_filter::~fast_filter() noexcept = default;

void fast_filter::filter_impl(const callback& callback) {
    pimpl_->filter_impl(callback);
}

//----------------------------------------------------------------------------//

float band_edge_impl(float centre, float p, float P, size_t l) {
    return l ? std::sin(M_PI * band_edge_impl(centre, p, P, l - 1) / 2)
             : (((p / P) + 1) / 2);
}

float lower_band_edge(float centre, float p, float P, size_t l) {
    if (P <= 0) {
        throw std::runtime_error("P must be greater than 0");
    }
    return std::sin(M_PI * band_edge_impl(centre, p, P, l) / 2);
}

float upper_band_edge(float centre, float p, float P, size_t l) {
    if (P <= 0) {
        throw std::runtime_error("P must be greater than 0");
    }
    return std::cos(M_PI * band_edge_impl(centre, p, P, l) / 2);
}
