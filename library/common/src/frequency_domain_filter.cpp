#include "common/frequency_domain_fiLter.h"
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

        for (auto& i : rbuf_) {
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

double band_edge_impl(double centre, double p, double P, size_t l) {
    return l ? std::sin(M_PI * band_edge_impl(centre, p, P, l - 1) / 2)
             : (((p / P) + 1) / 2);
}

double lower_band_edge(double centre, double p, double P, size_t l) {
    if (P <= 0) {
        throw std::runtime_error("P must be greater than 0");
    }
    return std::sin(M_PI * band_edge_impl(centre, p, P, l) / 2);
}

double upper_band_edge(double centre, double p, double P, size_t l) {
    if (P <= 0) {
        throw std::runtime_error("P must be greater than 0");
    }
    return std::cos(M_PI * band_edge_impl(centre, p, P, l) / 2);
}

double band_edge_frequency(int band, size_t bands, util::range<double> range) {
    return range.get_min() * std::pow(range.get_max() / range.get_min(),
                                      band / static_cast<double>(bands));
}

double compute_bandpass_magnitude(double frequency,
                                  util::range<double> range,
                                  double lower_edge_width,
                                  double upper_edge_width,
                                  size_t l) {
    if (frequency < range.get_min() - lower_edge_width ||
        range.get_max() + upper_edge_width <= frequency) {
        return 0;
    }

    const auto lower_p{frequency - range.get_min()};
    if (-lower_edge_width <= lower_p && lower_p < lower_edge_width) {
        return lower_band_edge(range.get_min(), lower_p, lower_edge_width, l);
    }

    const auto upper_p{frequency - range.get_max()};
    if (-upper_edge_width <= upper_p && upper_p < upper_edge_width) {
        return upper_band_edge(range.get_max(), upper_p, upper_edge_width, l);
    }

    return 1;
}

double compute_lopass_magnitude(double frequency,
                                double cutoff,
                                double width,
                                size_t l) {
    if (frequency < cutoff - width) {
        return 1;
    }
    if (cutoff + width <= frequency) {
        return 0;
    }
    return upper_band_edge(cutoff, frequency - cutoff, width, l);
}

double compute_hipass_magnitude(double frequency,
                                double cutoff,
                                double width,
                                size_t l) {
    if (frequency < cutoff - width) {
        return 0;
    }
    if (cutoff + width <= frequency) {
        return 1;
    }
    return lower_band_edge(cutoff, frequency - cutoff, width, l);
}
