#ifndef __HISSTOOLS_SPECTRUM__
#define __HISSTOOLS_SPECTRUM__

#include "HISSTools_FFT/HISSTools_FFT.h"

class Spectrum
{

public:
    
    enum SpectrumFormat {
        
        kSpectrumReal = 0,
        kSpectrumComplex = 1,
    };

	Spectrum(unsigned long maxFFTSize, SpectrumFormat format = kSpectrumReal)
	{
		// Check FFT Size is power of two for allocation
		
		maxFFTSize = 1 << log2(maxFFTSize);
		
		unsigned long requiredPointerSize = calcMaxBin(maxFFTSize, format);
		
		// Allocate memory assuming failure
		// This might need to be altered later to ensure 16 byte alignment!!
		
		mPointerSize = 0;
		mFFTSize = 0;
		mSamplingRate = 0;
		mFormat = format;
		
		mSpectrum.realp = new double[requiredPointerSize];
		mSpectrum.imagp = new double[requiredPointerSize];
		
		if (mSpectrum.realp && mSpectrum.imagp)
		{
			mPointerSize = requiredPointerSize;
			mFFTSize = maxFFTSize;
		}
	}
	
	~Spectrum()
	{
		delete[] mSpectrum.realp;
		delete[] mSpectrum.imagp;
	}
	
	bool copy(Spectrum *inSpectrum)
	{
		SpectrumFormat format = inSpectrum->getFormat();
		
		double *rIn = inSpectrum->getSpectrum()->realp;
		double *iIn = inSpectrum->getSpectrum()->imagp;
		double *rOut = inSpectrum->getSpectrum()->realp;
		double *iOut = inSpectrum->getSpectrum()->imagp;
		
		unsigned long FFTSize = inSpectrum->getFFTSize();
		unsigned long nBins = (format == kSpectrumReal) ? FFTSize >> 1 : FFTSize;
		
		// Attempt to set size and format
		
		if (setParams(FFTSize, format, inSpectrum->getSamplingRate()) == false)
			return false;
		
		// Copy spectrum
		
		for (unsigned long i = 0; i < nBins; i++)
		{
			rOut[i] = rIn[i];
			iOut[i] = iIn[i];
		}
				
		return true;
	}

private:
	
	unsigned long log2 (unsigned long in)
	{
		unsigned long temp = in;
		unsigned long out = 0;
		
		while (temp)
		{
			temp >>= 1;
			out++;		
		}
		
		if (in == 1 << (out - 1)) 
			return out - 1;
		else 
			return out;
	}
	
	
public:
	
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
	
	
	bool setFFTSize(unsigned long FFTSize)
	{
		return setParams(FFTSize, mFormat, mSamplingRate);
	}
	
	
	bool setFormat(SpectrumFormat format)
	{
		return setParams(mFFTSize, format, mSamplingRate);
	}
	
	void setSamplingRate(double samplingRate)
	{
		mSamplingRate = fabs(samplingRate);
	}
	
	
	FFT_SPLIT_COMPLEX_D *getSpectrum()
	{
		return &mSpectrum;
	}
	
	unsigned long getFFTSize()
	{
		return mFFTSize;
	}
	
	SpectrumFormat getFormat()
	{
		return mFormat;
	}
	
	double getSamplingRate()
	{
		return mSamplingRate;
	}
	
	static unsigned long calcMaxBin(unsigned long FFTSize, SpectrumFormat format)
	{
		switch (format)
		{
			case kSpectrumReal:
				return (FFTSize + 1) >> 1;
				
			case kSpectrumComplex:
				return FFTSize;
		}
		
		return 0;
	}

	
	unsigned long getMaxBin()
	{
		return calcMaxBin(mFFTSize, mFormat);
	}
	
	
private:
	
	// The Spectrum
	
	FFT_SPLIT_COMPLEX_D mSpectrum;
	
	// Parameters
	
	SpectrumFormat mFormat;
	
	unsigned long mFFTSize;
	unsigned long mPointerSize;
	
	// Sampling Rate (drawing/analysis etc.)
	
	double mSamplingRate;
};

#endif
