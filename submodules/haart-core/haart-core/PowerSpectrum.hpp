

#ifndef __HISSTOOLS_POWERSPECTRUM__
#define __HISSTOOLS_POWERSPECTRUM__


#include "HISSTools_FFT/HISSTools_FFT.h"
#include "Spectrum.hpp"


class PowerSpectrum
{
	
public:
	
    enum SpectrumFormat {
        
        kSpectrumNyquist = 0,
        kSpectrumFull = 1,
    };

	PowerSpectrum(unsigned long maxFFTSize, SpectrumFormat format = kSpectrumNyquist)
	{
		// Allocate memory
		
		unsigned long pointerSize = calcMaxBin(maxFFTSize, format);
		
		mSpectrum = new double[pointerSize];
		mFormat = format;
		mFFTSize = 0;
		mSamplingRate = 0.;
		
		if (mSpectrum) 
			mPointerSize = pointerSize;
		else
			mPointerSize = 0;
	}
	
	
	~PowerSpectrum()
	{
		delete[] mSpectrum;
	}

	
	bool copy(PowerSpectrum *inSpectrum)
	{
		SpectrumFormat format = inSpectrum->getFormat();
		
		double *in = inSpectrum->getSpectrum();
		double *out = getSpectrum();
		
		unsigned long FFTSize = inSpectrum->getFFTSize();
		unsigned long endBin = inSpectrum->getMaxBin();
		
		// Attempt to set size and format
		
		if (setParams(FFTSize, format, inSpectrum->getSamplingRate()) == false)
			return false;
		
		// Copy spectrum
		
		for (unsigned long i = 0; i < endBin; i++)
			out[i] = in[i];
						
		return true;
	}
	
	
	bool calcPowerSpectrum(Spectrum *inSpectrum, double scale = 0.)
	{
		FFT_SPLIT_COMPLEX_D in = *inSpectrum->getSpectrum();
        Spectrum::SpectrumFormat inFormat = inSpectrum->getFormat();
		
		double *spectrum = getSpectrum();
		unsigned long FFTSize = inSpectrum->getFFTSize();
		unsigned long startBin = 0;
		unsigned long endBin = calcMaxBin(FFTSize, mFormat);
		
		scale = scale == 0 ? 1 : scale;

		// Attempt to set size
		
		if (setFFTSize(FFTSize) == false)
			return false;
		
		// Do DC given a real only spectrum in (and set start/end bin
		
        if (inFormat == Spectrum::kSpectrumReal)
		{
			spectrum[0] = in.realp[0] * in.realp[0];
			startBin = 1;
			endBin = FFTSize >> 1;
		}
				
		// Loop over spectrum
		
		for (long i = startBin; i < endBin; i++)
			spectrum[i] = (in.realp[i] * in.realp[i] + in.imagp[i] * in.imagp[i]) * scale;
				
		// Clean up for real only spectrum in
		
		if (inFormat == Spectrum::kSpectrumReal)
		{
			// Do Nyquist
			
            spectrum[FFTSize >> 1] = (in.imagp[0] * in.imagp[0]) * scale;
		
			// Copy second half of spectrum
			
			if (mFormat == kSpectrumFull)
			{
				for (long i = 0; i < FFTSize; i++)
					spectrum[FFTSize - i] = spectrum[i];
			}
		}
		
		setSamplingRate(inSpectrum->getSamplingRate());
		
		return true;
	}
	
	
	static unsigned long calcMaxBin(unsigned long FFTSize, SpectrumFormat format)
	{
		switch (format)
		{				
			case kSpectrumNyquist:
				return ((FFTSize + 1) >> 1) + 1;
				
			case kSpectrumFull:
				return FFTSize;
		}
		
		return 0;
	}
	
	
	unsigned long getMaxBin()
	{
		return calcMaxBin(mFFTSize, mFormat);
	}
	
	
	bool setParams(unsigned long FFTSize, SpectrumFormat format, double samplingRate)
	{
		// Check pointer size

		if (calcMaxBin(FFTSize, format) > mPointerSize)
			return false;
		
		// Set parameters
		
		mFormat = format;
		mFFTSize = FFTSize;
		setSamplingRate(samplingRate);
		
		return true;
	}
	
	void setSamplingRate(double samplingRate)
	{
		mSamplingRate = fabs(samplingRate);
	}
	
	bool setFFTSize(unsigned long FFTSize)
	{
		return setParams(FFTSize, mFormat, mSamplingRate);
	}
	
	
	bool setFormat(SpectrumFormat format)
	{
		return setParams(mFFTSize, format, mSamplingRate);
	}
	
	
	double *getSpectrum()
	{
		return mSpectrum;
	}
	
	double getSamplingRate()
	{
		return mSamplingRate;
	}
	
	unsigned long getFFTSize()
	{
		return mFFTSize;
	}

	SpectrumFormat getFormat()
	{
		return mFormat;
	}
	
private:
	
	// The Spectrum 
	
	double *mSpectrum;
	
	// Parameters
	
	SpectrumFormat mFormat;
	
	unsigned long mFFTSize;
	unsigned long mPointerSize;
	
	// Sampling Rate (drawing/analysis etc.)
	
	double mSamplingRate;
};

#endif