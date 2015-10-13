#include "filters.h"

#include <cstring>

SelfDestructPlan::SelfDestructPlan(const fftwf_plan & plan): plan(plan) {}
SelfDestructPlan::~SelfDestructPlan() noexcept { fftwf_destroy_plan(plan); }
SelfDestructPlan::operator const fftwf_plan & () const {return plan;}

FastConvolution::FastConvolution (unsigned long FFT_LENGTH)
    : FFT_LENGTH(FFT_LENGTH)
    , CPLX_LENGTH(FFT_LENGTH / 2 + 1)
    , r2c_i (fftwf_alloc_real (FFT_LENGTH))
    , r2c_o (fftwf_alloc_complex (CPLX_LENGTH))
    , c2r_i (fftwf_alloc_complex (CPLX_LENGTH))
    , c2r_o (fftwf_alloc_real (FFT_LENGTH))
    , acplx (fftwf_alloc_complex (CPLX_LENGTH))
    , bcplx (fftwf_alloc_complex (CPLX_LENGTH))
    , r2c
      (   fftwf_plan_dft_r2c_1d
          (   FFT_LENGTH
          ,   r2c_i.get()
          ,   r2c_o.get()
          ,   FFTW_ESTIMATE
          )
      )
    , c2r
      (   fftwf_plan_dft_c2r_1d
          (   FFT_LENGTH
          ,   c2r_i.get()
          ,   c2r_o.get()
          ,   FFTW_ESTIMATE
          )
      )
{
}

std::vector<float> FastConvolution::convolve(const std::vector<float> & a, const std::vector<float> & b) {
    LOG_SCOPE;
    forward_fft (r2c, a, r2c_i, r2c_o, acplx);
    forward_fft (r2c, b, r2c_i, r2c_o, bcplx);
    Logger::log("forward ffts");

    memset(c2r_i.get(), 0, CPLX_LENGTH * sizeof(fftwf_complex));
    std::fill(c2r_o.get(), c2r_o.get() + FFT_LENGTH, 0);
    Logger::log("memsets");

    auto x = acplx.get();
    auto y = bcplx.get();
    auto z = c2r_i.get();

    // simd is for better people
    for (; z != c2r_i.get() + CPLX_LENGTH; ++x, ++y, ++z) {
        (*z) [0] += (*x) [0] * (*y) [0] - (*x) [1] * (*y) [1];
        (*z) [1] += (*x) [0] * (*y) [1] + (*x) [1] * (*y) [0];
    }

    Logger::log("cplx multiplies");

    fftwf_execute (c2r);

    Logger::log("back fft");

    return std::vector <float> (c2r_o.get(), c2r_o.get() + FFT_LENGTH);
}

void FastConvolution::forward_fft
(   const SelfDestructPlan & plan
,   const std::vector<float> & data
,   const fftwf_r & i
,   const fftwf_c & o
,   const fftwf_c & results
)
{
    LOG_SCOPE;

    memset(i.get(), 0, FFT_LENGTH * sizeof(float));
    memcpy(i.get(), data.data(), data.size() * sizeof(float));
    fftwf_execute (plan);
    memcpy(results.get(), o.get(), CPLX_LENGTH * sizeof(fftwf_complex));
}
