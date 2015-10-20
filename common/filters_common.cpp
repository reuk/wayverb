#include "filters_common.h"
#include "sinc.h"

#include <numeric>
#include <iostream>

using namespace std;

void Lopass::setParams (float co, float s)
{
    cutoff = co;
    sr = s;
}

void Hipass::setParams (float co, float s)
{
    cutoff = co;
    sr = s;
}

void Bandpass::setParams (float l, float h, float s)
{
    lo = l;
    hi = h;
    sr = s;
}

LopassWindowedSinc::LopassWindowedSinc
(   unsigned long inputLength
)
:   FastConvolution (KERNEL_LENGTH + inputLength - 1)
{

}

void LopassWindowedSinc::filter (vector <float> & data)
{
    data = convolve (kernel, data);
}

void LopassWindowedSinc::setParams
(   float co
,   float s
)
{
    auto i = lopass_sinc_kernel (s, co, KERNEL_LENGTH);
    copy (i.begin(), i.end(), kernel.begin());
}

HipassWindowedSinc::HipassWindowedSinc
(   unsigned long inputLength
)
:   FastConvolution (KERNEL_LENGTH + inputLength - 1)
{

}

void HipassWindowedSinc::filter (vector <float> & data)
{
    data = convolve (kernel, data);
}

void HipassWindowedSinc::setParams
(   float co
,   float s
)
{
    auto i = hipass_sinc_kernel (s, co, KERNEL_LENGTH);
    copy (i.begin(), i.end(), kernel.begin());
}

BandpassWindowedSinc::BandpassWindowedSinc
(   unsigned long inputLength
)
:   FastConvolution (KERNEL_LENGTH + inputLength - 1)
{

}

vector <float> BandpassWindowedSinc::bandpassKernel
(   float sr
,   float lo
,   float hi
)
{
    auto lop = lopass_sinc_kernel (sr, hi, 1 + KERNEL_LENGTH / 2);
    auto hip = hipass_sinc_kernel (sr, lo, 1 + KERNEL_LENGTH / 2);

    FastConvolution fc (KERNEL_LENGTH);
    return fc.convolve (lop, hip);
}

void BandpassWindowedSinc::filter
(   vector <float> & data
)
{
    data = convolve (kernel, data);
}

void BandpassWindowedSinc::setParams
(   float l
,   float h
,   float s
)
{
    auto i = bandpassKernel (s, l, h);
    copy (i.begin(), i.end(), kernel.begin());
}

void Biquad::onepass (vector <float> & data)
{
    double z1 = 0;
    double z2 = 0;

    for (auto && i : data)
    {
        double out = i * b0 + z1;
        z1 = i * b1 + z2 - a1 * out;
        z2 = i * b2 - a2 * out;
        i = out;
    }
}

void Biquad::setParams
(   double _b0
,   double _b1
,   double _b2
,   double _a1
,   double _a2
)
{
    b0 = _b0;
    b1 = _b1;
    b2 = _b2;
    a1 = _a1;
    a2 = _a2;
}

void Biquad::twopass (vector <float> & data)
{
    onepass (data);
    reverse (begin (data), end (data));
    onepass (data);
    reverse (begin (data), end (data));
}

void OnepassBandpassBiquad::setParams
(   float lo
,   float hi
,   float sr
)
{
    // From www.musicdsp.org/files/Audio-EQ-Cookbook.txt
    const double c = sqrt (lo * hi);
    const double omega = 2 * M_PI * c / sr;
    const double cs = cos (omega);
    const double sn = sin (omega);
    const double bandwidth = log2 (hi / lo);
    const double Q = sn / (log(2) * bandwidth * omega);
    const double alpha = sn * sinh (1 / (2 * Q));

    const double a0 = 1 + alpha;
    const double nrm = 1 / a0;

    Biquad::setParams
    (   nrm * alpha
    ,   nrm * 0
    ,   nrm * -alpha
    ,   nrm * (-2 * cs)
    ,   nrm * (1 - alpha)
    );
}

void OnepassBandpassBiquad::filter (vector <float> & data)
{
    onepass (data);
}

void TwopassBandpassBiquad::filter (vector <float> & data)
{
    twopass (data);
}

double getC (double co, double sr)
{
    const double wcT = M_PI * co / sr;
    return cos (wcT) / sin (wcT);
}

void LinkwitzRiley::setParams (float l, float h, float s)
{
    {
        const double c = getC (h, s);
        const double a0 = c * c + c * sqrt (2) + 1;
        lopass.setParams
        (   1 / a0
        ,   2 / a0
        ,   1 / a0
        ,   (-2 * (c * c - 1)) / a0
        ,   (c * c - c * sqrt (2) + 1) / a0
        );
    }
    {
        const double c = getC (l, s);
        const double a0 = c * c + c * sqrt (2) + 1;
        hipass.setParams
        (   (c * c) / a0
        ,   (-2 * c * c) / a0
        ,   (c * c) / a0
        ,   (-2 * (c * c - 1)) / a0
        ,   (c * c - c * sqrt (2) + 1) / a0
        );
    }
}

void LinkwitzRiley::filter (vector <float> & data)
{
    lopass.twopass (data);
    hipass.twopass (data);
}

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

