#include "frequency_domain/filter.h"

#include "plan.h"

#include <cmath>

namespace frequency_domain {

class filter::impl final {
public:
    using cbuf = buffer<fftwf_complex>;

    explicit impl(rbuf& rbuf)
            : rbuf_{rbuf}
            , cbuf_{rbuf.size() / 2 + 1}
            , fft_{fftwf_plan_dft_r2c_1d(
                      rbuf.size(), rbuf.data(), cbuf_.data(), FFTW_ESTIMATE)}
            , ifft_{fftwf_plan_dft_c2r_1d(
                      rbuf.size(), cbuf_.data(), rbuf.data(), FFTW_ESTIMATE)} {}

    void filter_impl(const filter::callback& callback) {
        //  Run forward fft, placing fft output into cbuf_.
        fftwf_execute(fft_);

        const auto rbuf_size = rbuf_.size();
        //  Modify magnitudes in the frequency domain.
        for (auto i = 0ul, end = cbuf_.size(); i != end; ++i) {
            const auto normalised_frequency = i / static_cast<float>(rbuf_size);
            auto& bin{cbuf_.data()[i]};
            auto& re{bin[0]};
            auto& im{bin[1]};
            const auto new_value =
                    callback(std::complex<float>{re, im}, normalised_frequency);
            re = new_value.real();
            im = new_value.imag();
        }

        //  Run inverse fft, placing ifft output back into owner.rbuf_.
        fftwf_execute(ifft_);

        //  Normalize the filter output.
        for (auto& i : rbuf_) {
            i /= rbuf_size;
        }
    }

private:
    rbuf& rbuf_;
    cbuf cbuf_;
    plan fft_;
    plan ifft_;
};

////////////////////////////////////////////////////////////////////////////////

filter::filter(size_t signal_length)
        : rbuf_{signal_length}
        , pimpl_{std::make_unique<impl>(rbuf_)} {}

filter::~filter() noexcept = default;

void filter::filter_impl(const callback& callback) {
    pimpl_->filter_impl(callback);
}

}  // namespace frequency_domain
