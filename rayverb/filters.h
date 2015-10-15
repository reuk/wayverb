#pragma once

#include "fftw3.h"

#include <array>
#include <vector>
#include <cmath>
#include <algorithm>
#include <iostream>

/// This namespace houses all of the machinery for multiband crossover
/// filtering.
namespace RayverbFiltering
{
    using namespace std;

    /// Interface for the most generic boring filter.
    class Filter
    {
    public:
        /// Given a vector of data, return a bandpassed version of the data.
        virtual void filter (vector <float> & data) = 0;
    };

    /// Interface for a plain boring hipass filter.
    class Hipass: public Filter
    {
    public:
        /// A hipass has mutable cutoff and samplerate.
        virtual void setParams (float co, float s);
        float cutoff, sr;
    };

    /// Interface for a plain boring bandpass filter.
    class Bandpass: public Filter
    {
    public:
        /// A hipass has mutable lopass, hipass, and samplerate.
        virtual void setParams (float l, float h, float s);
        float lo, hi, sr;
    };

    /// Performs the fastest offline convolution using fft wizardry.
    class FastConvolution
    {
    public:

        /// An fftconvolover has a constant length.
        /// This means you can reuse it for lots of different convolutions
        /// without reallocating memory, as long as they're all the same size.
        FastConvolution (unsigned long FFT_LENGTH);
        virtual ~FastConvolution();

        /// Convolve two data structures stogether.
        template <typename T, typename U>
        vector <float> convolve
        (   const T & a
        ,   const U & b
        )
        {
            forward_fft (r2c, a, r2c_i, r2c_o, acplx);
            forward_fft (r2c, b, r2c_i, r2c_o, bcplx);

            memset (c2r_i, 0, sizeof (fftwf_complex) * CPLX_LENGTH);
            memset (c2r_o, 0, sizeof (float) * FFT_LENGTH);

            fftwf_complex * x = acplx;
            fftwf_complex * y = bcplx;
            fftwf_complex * z = c2r_i;

            for (; z != c2r_i + CPLX_LENGTH; ++x, ++y, ++z)
            {
                (*z) [0] += (*x) [0] * (*y) [0] - (*x) [1] * (*y) [1];
                (*z) [1] += (*x) [0] * (*y) [1] + (*x) [1] * (*y) [0];
            }

            fftwf_execute (c2r);

            return vector <float> (c2r_o, c2r_o + FFT_LENGTH);
        }

    private:
        template <typename T>
        void forward_fft
        (   fftwf_plan & plan
        ,   const T & data
        ,   float * i
        ,   fftwf_complex * o
        ,   fftwf_complex * results
        )
        {
            fill (i, i + FFT_LENGTH, 0);
            copy (data.begin(), data.end(), i);
            fftwf_execute (plan);
            memcpy (results, o, sizeof (fftwf_complex) * CPLX_LENGTH);
        }

        const unsigned long FFT_LENGTH;
        const unsigned long CPLX_LENGTH = FFT_LENGTH / 2 + 1;

        float * r2c_i;
        fftwf_complex * r2c_o;
        fftwf_complex * c2r_i;
        float * c2r_o;
        fftwf_complex * acplx;
        fftwf_complex * bcplx;

        fftwf_plan r2c;
        fftwf_plan c2r;
    };

    /// An interesting windowed-sinc hipass filter.
    class HipassWindowedSinc: public Hipass, public FastConvolution
    {
    public:
        HipassWindowedSinc (unsigned long inputLength);

        /// Filter a vector of data.
        virtual void filter (vector <float> & data);
        virtual void setParams (float co, float s);
    private:
        static const auto KERNEL_LENGTH = 29;
        array <float, KERNEL_LENGTH> kernel;
    };

    /// An interesting windowed-sinc bandpass filter.
    class BandpassWindowedSinc: public Bandpass, public FastConvolution
    {
    public:
        BandpassWindowedSinc (unsigned long inputLength);

        /// Filter a vector of data.
        virtual void filter (vector <float> & data);
        virtual void setParams (float l, float h, float s);
    private:
        static const auto KERNEL_LENGTH = 29;

        /// Fetch a convolution kernel for a bandpass filter with the given
        /// paramters.
        static vector <float> bandpassKernel
        (   float sr
        ,   float lo
        ,   float hi
        );

        array <float, KERNEL_LENGTH> kernel;
    };

    /// A super-simple biquad filter.
    class Biquad
    {
    public:
        /// Run the filter foward over some data.
        void onepass (vector <float> & data);

        /// Run the filter forward then backward over some data.
        void twopass (vector <float> & data);

        void setParams
        (   double b0
        ,   double b1
        ,   double b2
        ,   double a1
        ,   double a2
        );
    private:
        double b0, b1, b2, a1, a2;
    };

    /// Simple biquad bandpass filter.
    class OnepassBandpassBiquad: public Bandpass, public Biquad
    {
    public:
        void setParams (float l, float h, float s);
        void filter (vector <float> & data);
    };

    /// Simple biquad bandpass filter.
    class TwopassBandpassBiquad: public OnepassBandpassBiquad
    {
        void filter (vector <float> & data);
    };

    /// A linkwitz-riley filter is just a linear-phase lopass and hipass
    /// coupled together.
    class LinkwitzRiley: public Bandpass
    {
    public:
        void setParams (float l, float h, float s);
        void filter (vector <float> & data);
    private:
        Biquad lopass, hipass;
    };

    /// Enum denoting available filter types.
    enum FilterType
    {   FILTER_TYPE_WINDOWED_SINC
    ,   FILTER_TYPE_BIQUAD_ONEPASS
    ,   FILTER_TYPE_BIQUAD_TWOPASS
    ,   FILTER_TYPE_LINKWITZ_RILEY
    };

    /// Given a filter type and a vector of vector of float, return the
    /// parallel-filtered and summed data, using the specified filtering method.
    void filter
    (   FilterType ft
    ,   vector <vector <vector <float>>> & data
    ,   float sr
    ,   float lo_cutoff
    );
}
