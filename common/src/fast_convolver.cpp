#include "common/fast_convolver.h"

#include "fftw/plan.h"

using fftwf::rbuf;
using cbuf = fftwf::buffer<fftwf_complex>;

class fast_convolver::impl final {
public:
    explicit impl(fast_convolver& owner, size_t fft_length)
            : fft_length(fft_length)
            , cplx_length(fft_length / 2 + 1)
            , r2c_o(cplx_length)
            , c2r_i(cplx_length)
            , c2r_o(fft_length)
            , acplx(cplx_length)
            , bcplx(cplx_length)
            , r2c(fftwf_plan_dft_r2c_1d(fft_length,
                                        owner.r2c_i.data(),
                                        r2c_o.data(),
                                        FFTW_ESTIMATE))
            , c2r(fftwf_plan_dft_c2r_1d(
                      fft_length, c2r_i.data(), c2r_o.data(), FFTW_ESTIMATE)) {}

    size_t get_fft_length() const { return fft_length; }

    void forward_fft_a() {
        fftwf_execute(r2c);
		acplx = r2c_o;
    }

    void forward_fft_b() {
        fftwf_execute(r2c);
		bcplx = r2c_o;
    }

    /// Preconditions:
    ///     acplx holds an fft of a signal
    ///     bcplx holds an fft of another signal
    aligned::vector<float> convolve_impl() {
		//	possibly not necessary
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
            i /= fft_length;
        }

        return ret;
    }

private:
    const size_t fft_length;
    const size_t cplx_length;

    cbuf r2c_o;
    cbuf c2r_i;
    rbuf c2r_o;
    cbuf acplx;
    cbuf bcplx;

    fftwf::plan r2c;
    fftwf::plan c2r;
};

//----------------------------------------------------------------------------//

fast_convolver::fast_convolver(size_t fft_length)
        : r2c_i(fft_length)
        , pimpl(std::make_unique<impl>(*this, fft_length)) {}

fast_convolver::~fast_convolver() noexcept = default;

size_t fast_convolver::get_fft_length() const {
    return pimpl->get_fft_length();
}

void fast_convolver::forward_fft_a() { pimpl->forward_fft_a(); }
void fast_convolver::forward_fft_b() { pimpl->forward_fft_b(); }

aligned::vector<float> fast_convolver::convolve_impl() {
    return pimpl->convolve_impl();
}
