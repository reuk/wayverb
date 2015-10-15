#include "filters.h"
#include "generic_functions.h"

#include <numeric>
#include <iostream>

using namespace std;

/// sinc t = sin (pi . t) / pi . t
template <typename T>
T sinc (const T & t)
{
    T pit = M_PI * t;
    return sin (pit) / pit;
}

/// Generate a convolution kernel for a lowpass sinc filter (NO WINDOWING!).
template <typename T>
vector <T> sincKernel (double cutoff, unsigned long length)
{
    if (! (length % 2))
        throw runtime_error ("Length of sinc filter kernel must be odd.");

    vector <T> ret (length);
    for (auto i = 0; i != length; ++i)
    {
        if (i == ((length - 1) / 2))
            ret [i] = 1;
        else
            ret [i] = sinc (2 * cutoff * (i - (length - 1) / 2.0));
    }
    return ret;
}

/// Generate a blackman window of a specific length.
template <typename T>
vector <T> blackman (unsigned long length)
{
    const auto a0 = 7938.0 / 18608.0;
    const auto a1 = 9240.0 / 18608.0;
    const auto a2 = 1430.0 / 18608.0;

    vector <T> ret (length);
    for (auto i = 0; i != length; ++i)
    {
        const auto offset = i / (length - 1.0);
        ret [i] =
        (   a0
        -   a1 * cos (2 * M_PI * offset)
        +   a2 * cos (4 * M_PI * offset)
        );
    }
    return ret;
}

/// Generate a windowed, normalized low-pass sinc filter kernel of a specific
/// length.
vector <float> lopassKernel (float sr, float cutoff, unsigned long length)
{
    auto window = blackman <float> (length);
    auto kernel = sincKernel <float> (cutoff / sr, length);
    transform
    (   begin (window)
    ,   end (window)
    ,   begin (kernel)
    ,   begin (kernel)
    ,   [] (auto i, auto j) { return i * j; }
    );
    normalize (kernel);
    return kernel;
}

/// Generate a windowed, normalized high-pass sinc filter kernel of a specific
/// length.
vector <float> hipassKernel (float sr, float cutoff, unsigned long length)
{
    auto kernel = lopassKernel (sr, cutoff, length);
    for (auto && i : kernel) i = -i;
    kernel [(length - 1) / 2] += 1;
    return kernel;
}

void RayverbFiltering::Hipass::setParams (float co, float s)
{
    cutoff = co;
    sr = s;
}

void RayverbFiltering::Bandpass::setParams (float l, float h, float s)
{
    lo = l;
    hi = h;
    sr = s;
}

RayverbFiltering::HipassWindowedSinc::HipassWindowedSinc
(   unsigned long inputLength
)
:   FastConvolution (KERNEL_LENGTH + inputLength - 1)
{

}

void RayverbFiltering::HipassWindowedSinc::filter (vector <float> & data)
{
    data = convolve (kernel, data);
}

void RayverbFiltering::HipassWindowedSinc::setParams
(   float co
,   float s
)
{
    auto i = hipassKernel (s, co, KERNEL_LENGTH);
    copy (i.begin(), i.end(), kernel.begin());
}

RayverbFiltering::BandpassWindowedSinc::BandpassWindowedSinc
(   unsigned long inputLength
)
:   FastConvolution (KERNEL_LENGTH + inputLength - 1)
{

}

vector <float> RayverbFiltering::BandpassWindowedSinc::bandpassKernel
(   float sr
,   float lo
,   float hi
)
{
    auto lop = lopassKernel (sr, hi, 1 + KERNEL_LENGTH / 2);
    auto hip = hipassKernel (sr, lo, 1 + KERNEL_LENGTH / 2);

    FastConvolution fc (KERNEL_LENGTH);
    return fc.convolve (lop, hip);
}

void RayverbFiltering::BandpassWindowedSinc::filter
(   vector <float> & data
)
{
    data = convolve (kernel, data);
}

void RayverbFiltering::BandpassWindowedSinc::setParams
(   float l
,   float h
,   float s
)
{
    auto i = bandpassKernel (s, l, h);
    copy (i.begin(), i.end(), kernel.begin());
}

void RayverbFiltering::Biquad::onepass (vector <float> & data)
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

void RayverbFiltering::Biquad::setParams
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

void RayverbFiltering::Biquad::twopass (vector <float> & data)
{
    onepass (data);
    reverse (begin (data), end (data));
    onepass (data);
    reverse (begin (data), end (data));
}

void RayverbFiltering::OnepassBandpassBiquad::setParams
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

void RayverbFiltering::OnepassBandpassBiquad::filter (vector <float> & data)
{
    onepass (data);
}

void RayverbFiltering::TwopassBandpassBiquad::filter (vector <float> & data)
{
    twopass (data);
}

double getC (double co, double sr)
{
    const double wcT = M_PI * co / sr;
    return cos (wcT) / sin (wcT);
}

void RayverbFiltering::LinkwitzRiley::setParams (float l, float h, float s)
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

void RayverbFiltering::LinkwitzRiley::filter (vector <float> & data)
{
    lopass.twopass (data);
    hipass.twopass (data);
}

void RayverbFiltering::filter
(   FilterType ft
,   vector <vector <vector <float>>> & data
,   float sr
,   float lo_cutoff
)
{
    unique_ptr <Bandpass> bp;

    switch (ft)
    {
    case FILTER_TYPE_WINDOWED_SINC:
        bp = unique_ptr <Bandpass>
        (   new BandpassWindowedSinc (data.front().front().size())
        );
        break;
    case FILTER_TYPE_BIQUAD_ONEPASS:
        bp = unique_ptr <Bandpass> (new OnepassBandpassBiquad());
        break;
    case FILTER_TYPE_BIQUAD_TWOPASS:
        bp = unique_ptr <Bandpass> (new TwopassBandpassBiquad());
        break;
    case FILTER_TYPE_LINKWITZ_RILEY:
        bp = unique_ptr <Bandpass> (new LinkwitzRiley());
        break;
    }

    for (auto && channel : data)
    {
        const vector <float> EDGES
            ({lo_cutoff, 175, 350, 700, 1400, 2800, 5600, 11200, 20000});

        for (auto i = 0; i != channel.size(); ++i)
        {
            bp->setParams (EDGES [i], EDGES [i + 1], sr);
            bp->filter (channel [i]);
        }
    }
}

RayverbFiltering::FastConvolution::FastConvolution (unsigned long FFT_LENGTH)
:   FFT_LENGTH (FFT_LENGTH)
,   r2c_i (fftwf_alloc_real (FFT_LENGTH))
,   r2c_o (fftwf_alloc_complex (CPLX_LENGTH))
,   c2r_i (fftwf_alloc_complex (CPLX_LENGTH))
,   c2r_o (fftwf_alloc_real (FFT_LENGTH))
,   acplx (fftwf_alloc_complex (CPLX_LENGTH))
,   bcplx (fftwf_alloc_complex (CPLX_LENGTH))
,   r2c
    (   fftwf_plan_dft_r2c_1d
        (   FFT_LENGTH
        ,   r2c_i
        ,   r2c_o
        ,   FFTW_ESTIMATE
        )
    )
,   c2r
    (   fftwf_plan_dft_c2r_1d
        (   FFT_LENGTH
        ,   c2r_i
        ,   c2r_o
        ,   FFTW_ESTIMATE
        )
    )
{

}

RayverbFiltering::FastConvolution::~FastConvolution()
{
    fftwf_destroy_plan (r2c);
    fftwf_destroy_plan (c2r);
    fftwf_free (r2c_i);
    fftwf_free (r2c_o);
    fftwf_free (acplx);
    fftwf_free (bcplx);
    fftwf_free (c2r_i);
    fftwf_free (c2r_o);
}
