#include "fast_convolution.h"

FastConvolution::FastConvolution(size_t FFT_LENGTH)
        : FFT_LENGTH(FFT_LENGTH)
        , CPLX_LENGTH(FFT_LENGTH / 2 + 1)
        , r2c_i(FFT_LENGTH)
        , r2c_o(CPLX_LENGTH)
        , c2r_i(CPLX_LENGTH)
        , c2r_o(FFT_LENGTH)
        , acplx(CPLX_LENGTH)
        , bcplx(CPLX_LENGTH)
        , r2c(fftwf_plan_dft_r2c_1d(
                  FFT_LENGTH, r2c_i.data(), r2c_o.data(), FFTW_ESTIMATE))
        , c2r(fftwf_plan_dft_c2r_1d(
                  FFT_LENGTH, c2r_i.data(), c2r_o.data(), FFTW_ESTIMATE)) {
}

