

#ifndef __HISSTOOLS_FFT_CPLUSPLUS__
#define __HISSTOOLS_FFT_CPLUSPLUS__


#include "Spectrum.hpp"
#include "HISSTools_FFT/HISSTools_FFT.h"

class HISSTools_FFT
{
	
public:
	
	HISSTools_FFT(long maxFFTSize)
	{
		long maxFFTSizeLog2 = log2(maxFFTSize);
		
		mFFTSetup = hisstools_create_setup_d(maxFFTSizeLog2);
		
		if (mFFTSetup)
			mMaxFFTSize = 1 << maxFFTSizeLog2;
		else 
			mMaxFFTSize = 0;
	}
	
	~HISSTools_FFT()
	{
		hisstools_destroy_setup_d(mFFTSetup);
	}
	
	
private:
	
	void timeToFullSpectrum (double *in, Spectrum *out, unsigned long nSamps, unsigned long FFTSizelog2)
	{
		FFT_SPLIT_COMPLEX_D spectrum = *out->getSpectrum();
		
		unsigned long FFTSize = 1 << FFTSizelog2;
		
		// Do real FFT

		timeToHalfSpectrum(in, out, nSamps, FFTSizelog2);
		
		// Copy real nyquist value and zero imaginary at DC at nyquist
		
		spectrum.realp[FFTSize >> 1] = spectrum.imagp[0];	
		spectrum.imagp[FFTSize >> 1] = 0.;
		spectrum.imagp[0] = 0.;
				
		// Copy second half of spectrum from first with conjugation

		for (unsigned long i = (FFTSize >> 1) + 1; i < FFTSize; i++)
		{
			spectrum.realp[i] =  spectrum.realp[FFTSize - i];
			spectrum.imagp[i] = -spectrum.imagp[FFTSize - i];
		}
	}
	
	
	void timeToHalfSpectrum(double *in, Spectrum *out, unsigned long nSamps, unsigned long FFTSizelog2)
	{
		FFT_SPLIT_COMPLEX_D spectrum = *out->getSpectrum();
		
		// Take the real fft
		
		hisstools_unzip_zero_d(in, &spectrum, nSamps, FFTSizelog2);
		hisstools_rfft_d(mFFTSetup, &spectrum, FFTSizelog2);
		
		// Scale
		
		for (unsigned long i = 0; i < (1 << (FFTSizelog2 - 1)); i++)
		{
			spectrum.realp[i] *= 0.5;
			spectrum.imagp[i] *= 0.5;		
		}
	}
	
	
public:
	
	static unsigned long log2(unsigned long value)
	{
		unsigned long bitShift = value;
		unsigned long bitCount = 0;
		
		while (bitShift)
		{
			bitShift >>= 1U;
			bitCount++;
		}
		
		if (value == 1U << (bitCount - 1U))
			return bitCount - 1U;
		else 
			return bitCount;
	}
	
	static unsigned long nextPowerOf2(unsigned long in)
	{
		return 1 << (log2(in));
	}
	
	bool timeToSpectrum(double *in, Spectrum *out, unsigned long nSamps, unsigned long FFTSize, double samplingRate = 0.0)
	{
		// Get FFT size log 2
		
        unsigned long FFTSizelog2 = log2(FFTSize ? FFTSize : nSamps);
		FFTSize = 1 << FFTSizelog2;

		// Sanity Check / Attempt to set size
		
		if (nSamps > FFTSize || FFTSize > mMaxFFTSize || out->setFFTSize(FFTSize) == false)
			return false;
		
		// Transform
		
		if (out->getFormat() == Spectrum::kSpectrumReal)
			timeToHalfSpectrum(in, out, nSamps, FFTSizelog2);
		else 
			timeToFullSpectrum(in, out, nSamps, FFTSizelog2);
		
		out->setSamplingRate(samplingRate);
		
		return true;
	}
	
    bool timeToSpectrum(double *in, Spectrum *out, unsigned long nSamps, double samplingRate = 0.0)
    {
        return timeToSpectrum(in, out, nSamps, nSamps, samplingRate);
    }

	
	bool spectrumToTime(double *out, Spectrum *spectrumIn)
	{
		FFT_SPLIT_COMPLEX_D *spectrum = spectrumIn->getSpectrum();
        Spectrum::SpectrumFormat inFormat = spectrumIn->getFormat();
		
		unsigned long FFTSize = spectrumIn->getFFTSize();
		unsigned long FFTSizelog2 = log2(FFTSize);
		
		double scale = 1.0 / (FFTSize);
		double DCStore = 0.0;
		
		// Sanity Check
		
		if (FFTSize > mMaxFFTSize)
			return false;
		
		// Temporarily move nyquist values if necessary
		
		if (inFormat == Spectrum::kSpectrumComplex)
		{
			DCStore = spectrum->imagp[0];
			spectrum->imagp[0] = spectrum->realp[FFTSize >> 1];
		}
		
		// Convert to time domain 
		
		hisstools_rifft_d(mFFTSetup, spectrum, FFTSizelog2);
		hisstools_zip_d(spectrum, out, FFTSizelog2);
		
		if (inFormat == Spectrum::kSpectrumComplex)
			spectrum->imagp[0] = DCStore;
		
		// Scale
		
		for (unsigned long i = 0; i < FFTSize; i++)
			out[i] *= scale;
		
		return true;
	}	
	
	
protected:
	
	// FFT Setup
	
	FFT_SETUP_D mFFTSetup;
	
	// Maximum FFT Size
	
	unsigned long mMaxFFTSize;
};


#endif
