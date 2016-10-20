#include "frequency_domain/convolver.h"

#include "plan.h"

namespace frequency_domain {

class convolver::impl final {
public:
    using rbuf = rbuf;
    using cbuf = buffer<fftwf_complex>;

    explicit impl(convolver& owner, size_t fft_length)
            : fft_length_{fft_length}
            , cplx_length_{fft_length / 2 + 1}
            , r2c_o_{cplx_length_}
            , c2r_i_{cplx_length_}
            , c2r_o_{fft_length_}
            , acplx_{cplx_length_}
            , bcplx_{cplx_length_}
            , r2c_{fftwf_plan_dft_r2c_1d(fft_length,
                                         owner.r2c_i_.data(),
                                         r2c_o_.data(),
                                         FFTW_ESTIMATE)}
            , c2r_{fftwf_plan_dft_c2r_1d(fft_length,
                                         c2r_i_.data(),
                                         c2r_o_.data(),
                                         FFTW_ESTIMATE)} {}

    size_t get_fft_length() const { return fft_length_; }

    void forward_fft_a() {
        fftwf_execute(r2c_);
        acplx_ = r2c_o_;
    }

    void forward_fft_b() {
        fftwf_execute(r2c_);
        bcplx_ = r2c_o_;
    }

    /// Preconditions:
    ///     acplx holds an fft of a signal
    ///     bcplx holds an fft of another signal
    std::vector<float> convolve_impl() {
        //	possibly not necessary
        c2r_i_.zero();
        c2r_o_.zero();

        auto x = acplx_.begin();
        auto y = bcplx_.begin();
        auto z = c2r_i_.begin();

        for (; z != c2r_i_.end(); ++x, ++y, ++z) {
            (*z)[0] += (*x)[0] * (*y)[0] - (*x)[1] * (*y)[1];
            (*z)[1] += (*x)[0] * (*y)[1] + (*x)[1] * (*y)[0];
        }

        fftwf_execute(c2r_);

        std::vector<float> ret(c2r_o_.begin(), c2r_o_.end());

        for (auto& i : ret) {
            i /= fft_length_;
        }

        return ret;
    }

private:
    const size_t fft_length_;
    const size_t cplx_length_;

    cbuf r2c_o_;
    cbuf c2r_i_;
    rbuf c2r_o_;
    cbuf acplx_;
    cbuf bcplx_;

    plan r2c_;
    plan c2r_;
};

//----------------------------------------------------------------------------//

convolver::convolver(size_t fft_length)
        : r2c_i_{fft_length}
        , pimpl_{std::make_unique<impl>(*this, fft_length)} {}

convolver::~convolver() noexcept = default;

size_t convolver::get_fft_length() const { return pimpl_->get_fft_length(); }

void convolver::forward_fft_a() { pimpl_->forward_fft_a(); }
void convolver::forward_fft_b() { pimpl_->forward_fft_b(); }

std::vector<float> convolver::convolve_impl() {
    return pimpl_->convolve_impl();
}

}  // namespace frequency_domain
