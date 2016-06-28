
#ifndef __HIRT_CORE__
#define __HIRT_CORE__

#include "HISSTools_FFT.hpp"
#include "DeconvolutionSpecification.h"
#include "Spectrum.hpp"

#include <complex>
#include <stdint.h>


#define HIRT_DB_MIN -500


// FIX - sort this out


class HIRT_Core : public HISSTools_FFT
{	
public:

    enum DeconvolutionMode { kModeRegularised = 0, kModeClip = 1, kModeFilter = 2 };
    
	HIRT_Core(uintptr_t maxFFTSize) : HISSTools_FFT(maxFFTSize)
	{
		setDeconvolutionParams(kModeRegularised, 0.0, 0.0);
		setDeconvolutionCustomFilter();
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////// Frequency Specified Power Array //////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	class FrequencyDependentPowerSpecification
	{
        enum TraversalDirection {kUp, kDown};
	
	private:
		
		// Parameters
		
        std::vector <double> mSpecifiers;
		
		double mSpecifierOffset;
				
		// Temporary Working Variables
		
		TraversalDirection mDirection;
		
		uintptr_t mFFTSize;
		uintptr_t mListPos;
		uintptr_t mBin;
		
		double mFreqMultipler;
		double mPrevLogFreq;
		double mNextLogFreq;
		
		double mGradient;
		double mOffset;
    
	public:
		
		FrequencyDependentPowerSpecification()
		{
			mSpecifierOffset = 0.0;
		}
		
        void setSpecification(std::vector <double> *specifiers)
		{
            mSpecifiers = *specifiers;
		}
        
		void setSpecification(double *specifiers, unsigned int nValues)
		{
            mSpecifiers.clear();
            
			for (unsigned int i = 0; i < nValues; i++)
				mSpecifiers.push_back(specifiers[i]);
		}
		
		void setOffset(double offset)
		{
			mSpecifierOffset = offset;
		}
		
    private:
        
        double getFirstPowerSpecified()
        {
            unsigned long size = mSpecifiers.size();

            if (!size)
                return dbToPow(mSpecifierOffset);
            
            return (size == 1) ? dbToPow(mSpecifiers[0] + mSpecifierOffset) : dbToPow(mSpecifiers[1] + mSpecifierOffset);
        }
       
    public:
        
		double getFirstPowerSpecified(uintptr_t FFTSize, double sampleRate)
		{
            unsigned long size = mSpecifiers.size();
            
			mDirection = kUp;
			
			mFFTSize = FFTSize;
			mListPos = 0;
			mBin = 0;
			
			mFreqMultipler =  sampleRate / (double) FFTSize;
			mPrevLogFreq = -HUGE_VAL;
			mNextLogFreq = size ? log(mSpecifiers[0]) : 0.0;
			
			mGradient = 0;
			mOffset =  size > 1 ? mSpecifiers[1] : 0.0;
			
			return getFirstPowerSpecified();
		}
		
		double getNextPowerSpecified()
		{
            unsigned long size = mSpecifiers.size();
            
			double binLogFreq;
			
			// A single or dual value
			
			if (size <= 3)
				return getFirstPowerSpecified();
			
			// Search for the point in the list of freq / db duples and calculate new line if relevant
			
			switch (mDirection)
			{
				case kUp:
					
					binLogFreq = log(++mBin * mFreqMultipler);
					
					if (binLogFreq > mNextLogFreq)
					{
                        // FIX - this is unsafe
                        // FIX - check also the original
                        
						for (; binLogFreq > mNextLogFreq && mListPos < (size >> 1); mListPos++, mPrevLogFreq = mNextLogFreq, mNextLogFreq = log(mSpecifiers[mListPos << 1]));
						
                        // Check for end of list else calculate new line
                        
						if (mListPos == (size >> 1))
						{
							mGradient = 0;
							mOffset = mSpecifiers[(mListPos << 1) - 1];
							mNextLogFreq = HUGE_VAL;
						}
						else 
						{
							mGradient = (mSpecifiers[(mListPos << 1) + 1] - mSpecifiers[(mListPos << 1) - 1]) / (mNextLogFreq - mPrevLogFreq);
							mOffset = mSpecifiers[(mListPos << 1) - 1] - (mPrevLogFreq * mGradient);
						}
					}
					break;
					
				case kDown:
					
					binLogFreq = log(--mBin * mFreqMultipler);
					
					if (binLogFreq < mNextLogFreq)
					{
						for (; binLogFreq < mNextLogFreq && mListPos > 0; mListPos--, mPrevLogFreq = mNextLogFreq, mNextLogFreq = log(mSpecifiers[mListPos << 1]));
					
                        // Check for end of list else calculate new line

						if (mListPos == 0)
						{
							mGradient = 0;
							mOffset = mSpecifiers[1];
							mNextLogFreq = -HUGE_VAL;
						}
						else 
						{
							mGradient = (mSpecifiers[(mListPos << 1) - 1] - mSpecifiers[(mListPos << 1) + 1]) / (mNextLogFreq - mPrevLogFreq);
							mOffset = mSpecifiers[(mListPos << 1) + 1] - (mPrevLogFreq * mGradient);
						}
					}
					break;
			}
			
			// Check for direction switch (post switch on nyquist bin)
			
			if (mBin == (mFFTSize >> 1))
			{
				mDirection = kDown;	
				
				double tempLogFreq = mPrevLogFreq;
				mPrevLogFreq = mNextLogFreq;
				mNextLogFreq = tempLogFreq;
				
				mOffset = mSpecifiers[(mListPos << 1) + 1] - (mPrevLogFreq * mGradient);
			}
			
			// Calculate value
			
			return dbToPow(binLogFreq * mGradient + mOffset + mSpecifierOffset);		
		}
		
		double getNyquistPowerSpecified()
		{
            unsigned long size = mSpecifiers.size();

            // FIX - this does not work
            
			uintptr_t listPos;
			double prevLogFreq;
			double nextLogFreq = HUGE_VAL;
			
			// A single or dual value
			
			if (size <= 3)
				return getFirstPowerSpecified();
			
			// Search for the point in the list of freq / db duples and calculate line
			
			double binLogFreq = log((mFFTSize >> 1) * mFreqMultipler);
			
			for (listPos = (size >> 1) - 1, prevLogFreq = log(mSpecifiers[listPos << 1]); binLogFreq < nextLogFreq && listPos > 0; listPos--, prevLogFreq = nextLogFreq, nextLogFreq = log(mSpecifiers[listPos << 1]));
			
			if (mListPos == 0)
				return dbToPow(mSpecifiers[1]);
			
			// Calculate the line
			
			double gradient = (mSpecifiers[(listPos << 1) - 1] - mSpecifiers[(listPos << 1) + 1]) / (nextLogFreq - prevLogFreq);
			double offset = mSpecifiers[(listPos << 1) + 1] - (prevLogFreq * gradient);
			
			// Calculate value
			
			return dbToPow(binLogFreq * gradient + offset + mSpecifierOffset);		
		}
	};
		
private:
		
	DeconvolutionMode mDeconvolveMode;
	double mDeconvolvePhase;
	double mDeconvolveDelay;
	
	double *mCustomFilter;
	uintptr_t mCustomFilterLength;
	bool mCustomFilterChangePhase;
	
	FrequencyDependentPowerSpecification mFilterSpecification;
	FrequencyDependentPowerSpecification mRangeSpecification;
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////// Parameters /////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
public:
	
    void setDeconvolutionSpecification(DeconvolutionSpecification /*specification*/)
    {
    }
    
	void setDeconvolutionParams(DeconvolutionMode mode, double phase, double delay)
	{
		mDeconvolveMode = mode;
		mDeconvolvePhase = phase;
		mDeconvolveDelay = delay;
	}
	
	void setDeconvolutionCustomFilter(double *filter, uintptr_t filterLength, bool filterChangePhase = false)
	{
		mCustomFilter = filter;
		mCustomFilterLength = filterLength;
		mCustomFilterChangePhase = filterChangePhase;
	}
	
	void setDeconvolutionCustomFilter()
	{
		setDeconvolutionCustomFilter(NULL, 0);
	}
	
	void setFilterSpecification(double *specifiers, unsigned int nValues)
	{
		mFilterSpecification.setSpecification(specifiers, nValues);
	}
	
    void setFilterSpecification(std::vector <double> *specifiers)
	{
		mFilterSpecification.setSpecification(specifiers);
	}
    
	void setFilterSpecOffset(double offset)
	{
		mFilterSpecification.setOffset(offset);
	}
	
	void setRangeSpecification(double *specifiers, unsigned int nValues)
	{
		mRangeSpecification.setSpecification(specifiers, nValues);
	}
	
    void setRangeSpecification(std::vector <double> *specifiers)
	{
		mRangeSpecification.setSpecification(specifiers);
	}
    
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////// Utility //////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
public:
	
	static __inline double dbToA (double db)
	{
		return pow (10.0, db / 20.); 
	}
	
	
	static __inline double dbToPow (double db)
	{
		return pow (10.0, db / 10.); 
	}
	
	
	
	static __inline double aToDb (double a)
	{
		double db;
		
		if (!a) 
			return HIRT_DB_MIN;
		
		db = 20. * log10(a);
		
		if (db < HIRT_DB_MIN) 
			db = HIRT_DB_MIN;
		
		return db;
	}
	
	
	static __inline double powToDb (double pow)
	{
		double db;
		
		if (!pow) 
			return HIRT_DB_MIN;
		
		db = 10. * log10(pow);
		
		if (db < HIRT_DB_MIN) 
			db = HIRT_DB_MIN;
		
		return db;
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////// Conformity Checks /////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
    bool spectraConform(Spectrum *out, Spectrum *in1, Spectrum *in2, Spectrum *in3, Spectrum::SpectrumFormat *format, uintptr_t *FFTSize, uintptr_t *beg = NULL, uintptr_t *end = NULL)
	{
		double samplingRate = in1->getSamplingRate();
		
		*format = in1->getFormat();
		*FFTSize = in1->getFFTSize();
		
		if (*FFTSize == 0)
			return false;
		
		if (beg != NULL)
			*beg = *format == Spectrum::Spectrum::kSpectrumReal ? 1 : 0;
		if (end != NULL)
			*end = *format == Spectrum::Spectrum::kSpectrumReal ? (*FFTSize) >> 1 : (*FFTSize);
		
		if (in2 != NULL && (in2->getFormat() != *format || in2->getFFTSize() != *FFTSize || in2->getSamplingRate() != samplingRate))
			return false;
		
		if (in3 != NULL && (in2->getFormat() != *format || in2->getFFTSize() != *FFTSize || in2->getSamplingRate() != samplingRate))
			return false;
		
		if (out->setParams(*FFTSize, *format, samplingRate) == false)
			return false;
		
		return true;
	}
	
    bool spectraConform(Spectrum *out, Spectrum *in1, Spectrum *in2, Spectrum::SpectrumFormat *format, uintptr_t *FFTSize, uintptr_t *beg = NULL, uintptr_t *end = NULL)
	{
		return spectraConform(out, in1, in2, NULL, format, FFTSize, beg, end);
	}
	
    bool spectraConform(Spectrum *out, Spectrum *in1, Spectrum::SpectrumFormat *format, uintptr_t *FFTSize, uintptr_t *beg = NULL, uintptr_t *end = NULL)
	{
		return spectraConform(out, in1, NULL, NULL, format, FFTSize, beg, end);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////// Spectrum Pointers Helper //////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	void getSpectrum(Spectrum *in, double **real, double **imag)
	{
		*real = in->getSpectrum()->realp;
		*imag = in->getSpectrum()->imagp;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////// Power Calculation Helper //////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
private:
	
	double calcPower(double real)
	{
		return (real * real);
	}
	
	double calcPower(double real, double imag)
	{
		return (real * real) + (imag * imag);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////// Phase Routines (Private) : Out-of-place ///////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
private:
	
	bool calcMinimumPhaseComponents(Spectrum *out, Spectrum *in)
	{
		double *rOut, *iOut, *rIn1, *iIn1;
		uintptr_t FFTSize, FFTSizelog2, i;
		Spectrum::SpectrumFormat format;
		
		double scale;
		double minPower = dbToPow(-3000.0);
		double power1;
		double power2;
		
		getSpectrum(in, &rIn1, &iIn1);
		getSpectrum(out, &rOut, &iOut);
		
		if (spectraConform(out, in, &format, &FFTSize) == false)
			return false;

		if (FFTSize > mMaxFFTSize)
			return false;

		FFTSizelog2 = log2(FFTSize);
		scale = 1.0 / FFTSize;
				
		if (format == Spectrum::kSpectrumComplex)
		{
			// Take Log of Power Spectrum
			
			for	(i = 0; i < FFTSize; i++)
			{				
				power1 = calcPower(rIn1[i], iIn1[i]);
				rOut[i] = 0.5 * log((power1 < minPower) ? minPower : power1);
				iOut[i] = 0.0;
			}
			
			// Do Complex iFFT
			
			hisstools_ifft_d(mFFTSetup, out->getSpectrum(), FFTSizelog2);
			
			// Double Causal Values / Zero Non-Casual Values / Scale All Remaining
			
			for (i = 1; i < (FFTSize >> 1); i++)
			{
				rOut[i] += rOut[i];
				iOut[i] += iOut[i];
			}
			for (i = (FFTSize >> 1) + 1; i < FFTSize; i++)
			{
				rOut[i] = 0.;
				iOut[i] = 0.;
			}
			for (i = 0; i < (FFTSize >> 1) + 1; i++)
			{
				rOut[i] *= scale;
				iOut[i] *= scale;
			}

			// Forward Complex FFT
			
			hisstools_fft_d(mFFTSetup, out->getSpectrum(), FFTSizelog2);
		}
		else 
		{
            // FIX - scaling is incorrect here (compensate for 2x each forward real)
            
			// Take Log of Power Spectrum
			
			power1 = calcPower(rIn1[0]);
			power2 = calcPower(iIn1[0]);
			rOut[0] = 0.5 * log((power1 < minPower) ? minPower : power1);
			iOut[0] = 0.5 * log((power2 < minPower) ? minPower : power2);
			
			for	(i = 1; i < (FFTSize >> 1); i++)
			{
				power1 = calcPower(rIn1[i], iIn1[i]);
				
				rOut[i] = 0.5 * log((power1 < minPower) ? minPower : power1);
				iOut[i] = 0.;
			}
			
			// Do Real iFFT
			
			hisstools_rifft_d(mFFTSetup, out->getSpectrum(), FFTSizelog2);
			
			// Double Causal Values / Zero Non-Casual Values / Scale All Remaining

			rOut[0] *= scale;
			iOut[0] = (iOut[0] + iOut[0]) * scale;
            
			for (i = 1; i < (FFTSize >> 2); i++)
			{
				rOut[i] = (rOut[i] + rOut[i]) * scale;
				iOut[i] = (iOut[i] + iOut[i]) * scale;
			}
			rOut[(FFTSize >> 2) + 0] *= scale;
			iOut[(FFTSize >> 2) + 0] = 0.;
			for (i = (FFTSize >> 2) + 1; i < (FFTSize >> 1); i++)
			{
				rOut[i] = 0.;
				iOut[i] = 0.;
			}
			
			// Forward Real FFT
			
			hisstools_rfft_d(mFFTSetup, out->getSpectrum(), FFTSizelog2);
		}
								   
		return true;
	}

	bool convertToMinimumPhase(Spectrum *out, Spectrum *in)
	{
		double *rOut, *iOut, *rIn1, *iIn1;
		uintptr_t FFTSize, beg, end;		
		Spectrum::SpectrumFormat format;
			
		getSpectrum(in, &rIn1, &iIn1);
		getSpectrum(out, &rOut, &iOut);
		
		if (spectraConform(out, in, &format, &FFTSize, &beg, &end) == false)
			return false;
		
		calcMinimumPhaseComponents(out, in);
				
		if (format == Spectrum::Spectrum::kSpectrumReal)
		{
			rOut[0] = exp(rOut[0]);
			iOut[0] = exp(iOut[0]);
		}
		for (uintptr_t i = beg; i < end; i++)
		{			
			std::complex <double> c = std::exp(std::complex <double> (rOut[i], iOut[i]));
			
			rOut[i] = std::real(c);
			iOut[i] = std::imag(c);
		}
		
		return true;
	}
	
	bool convertToMixedPhase(Spectrum *out, Spectrum *in, double phase, bool zeroCenter)
	{
		double *rOut, *iOut, *rIn1, *iIn1;
		uintptr_t FFTSize, beg, end;		
		Spectrum::SpectrumFormat format;
		
		double linphaseMul;
		double minphaseMul;	
		double interpPhase;
		double amp;
		
		getSpectrum(in, &rIn1, &iIn1);
		getSpectrum(out, &rOut, &iOut);
		
        if (spectraConform(out, in, &format, &FFTSize, &beg, &end) == false)
            return false;
        
		phase = phase < 0 ? 0.0 : phase;
		phase = phase > 1 ? 1.0 : phase;
		
		// N.B. - induce a delay of -1 sample for anything over linear end avoid wraparound end the first sample
		
		minphaseMul = 1 - (2 * phase);
		linphaseMul = zeroCenter == true ? 0.0 : (phase <= 0.5 ? -(2 * M_PI * phase) : (-2 * M_PI * (phase - 1.0 / FFTSize)));
		
		calcMinimumPhaseComponents(out, in);
				
		if (format == Spectrum::kSpectrumReal)
		{
			rOut[0] = exp(rIn1[0]);
			iOut[0] = exp(iIn1[0]);
		}
		for (uintptr_t i = beg; i < end; i++)
		{
			amp = exp(rIn1[i]);
			interpPhase = linphaseMul * i + minphaseMul * iIn1[i];
			
			rOut[i] = amp * cos(interpPhase);
			iOut[i] = amp * sin(interpPhase);
		}
		
		return true;
	}	
	
	bool convertToZeroPhase(Spectrum *out, Spectrum *in)
	{
		double *rOut, *iOut, *rIn1, *iIn1;
		uintptr_t FFTSize, beg, end;		
		Spectrum::SpectrumFormat format;
		
		getSpectrum(in, &rIn1, &iIn1);
		getSpectrum(out, &rOut, &iOut);
		
		if (spectraConform(out, in, &format, &FFTSize, &beg, &end) == false)
			return false;
				
		if (format == Spectrum::kSpectrumReal)
		{
			rOut[0] = sqrt(calcPower(rIn1[0]));
			iOut[0] = sqrt(calcPower(iIn1[0]));
		}
		for (uintptr_t i = beg; i < end; i++)
		{			
			rOut[i] = sqrt(calcPower(rIn1[i], iIn1[i]));
			iOut[i] = 0.0;
		}
		
		return true;
	}
	
	bool convertToLinearPhase(Spectrum *out, Spectrum *in)
	{
		if (convertToZeroPhase(out, in) == false)
			return false;
		return delaySpectrum(out, out->getFFTSize() / 2.0);
	}
	
	bool convertToNCMaximumPhase(Spectrum *out, Spectrum *in)
	{		
		if (convertToMinimumPhase(out, in) == false)
			return false;
		return timeReverse(out);
	}
	
	bool convertToMaximumPhase(Spectrum *out, Spectrum *in)
	{
		if (convertToNCMaximumPhase(out, in) == false)
			return false;
		return delaySpectrum(out, -1.0);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////// Phase Routines (Private) : In-place /////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	bool calcMinimumPhaseComponents(Spectrum *in)
	{
		return calcMinimumPhaseComponents(in, in);
	}
	
	bool convertToMinimumPhase(Spectrum *in)
	{
		return convertToMinimumPhase(in, in);
	}

	bool convertToMixedPhase(Spectrum *in, double phase, bool zeroCenter)
	{
		return convertToMixedPhase(in, in, phase, zeroCenter);
	}
	
	bool convertToZeroPhase(Spectrum *in)
	{
		return convertToZeroPhase(in, in);
	}
	
	bool convertToLinearPhase(Spectrum *in)
	{
		return convertToLinearPhase(in, in);
	}
	
	bool convertToNCMaximumPhase(Spectrum *in)
	{		
		return convertToNCMaximumPhase(in, in);
	}
	
	bool convertToMaximumPhase(Spectrum *in)
	{
		return convertToMaximumPhase(in, in);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////// Main Phase Routines (Public) ////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
public:
		
	bool changePhase(Spectrum *out, Spectrum *in, double phase, bool zeroCenter = false)
	{
		if (phase == 0.0)
			return convertToMinimumPhase(out, in);
		
		if (phase == 0.5)
		{
			if (zeroCenter)
				return convertToZeroPhase(out, in);
			else
				return convertToLinearPhase(out, in);
		}
		
		if (phase == 1.0)
		{
			if (zeroCenter)
				return convertToNCMaximumPhase(out, in);
			else
				return convertToMaximumPhase(out, in);
		}
		
		return convertToMixedPhase(out, in, phase, zeroCenter);
	}
	
	bool changePhase(Spectrum *in, double phase, bool zeroCenter = false)
	{
		return changePhase(in, in, phase, zeroCenter);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////// Convolution ///////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
public:
	
	bool convolve(Spectrum *out, Spectrum *in1, Spectrum *in2)
	{
		double *rOut, *iOut, *rIn1, *iIn1, *rIn2, *iIn2;
		Spectrum::SpectrumFormat format;
		uintptr_t beg, end, FFTSize;		
		
		getSpectrum(out, &rOut, &iOut);
		getSpectrum(in1, &rIn1, &iIn1);
		getSpectrum(in2, &rIn2, &iIn2);
		
		if (spectraConform(out, in1, in2, &format, &FFTSize, &beg, &end) == false)
			return false;
		
		if (format == Spectrum::kSpectrumReal)
		{		
			rOut[0] = rIn1[0] * rIn2[0];
			iOut[0] = iIn1[0] * iIn2[0];
		}
		for (uintptr_t i = beg; i < end; i++)
		{
			std::complex <double> c = std::complex <double> (rIn1[i], iIn1[i]) * std::complex <double>(rIn2[i], iIn2[i]);
			
			rOut[i] = std::real(c);
			iOut[i] = std::imag(c);
		}
		
		return true;
	}
	
	void convolve(Spectrum *io, Spectrum *in)
	{
		convolve(io, io, in);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////////////////// Deconvolution (Variable Phase Filter) ////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	bool deconvolveWithFilter(Spectrum *out, Spectrum *in1, Spectrum *in2, Spectrum *filter)
	{
		double *rOut, *iOut, *rIn1, *iIn1, *rIn2, *iIn2, *rIn3, *iIn3;
		Spectrum::SpectrumFormat format;
		uintptr_t beg, end, FFTSize;		
		std::complex <double> c1, c2, c3;
		double multiplier;
		
		getSpectrum(out, &rOut, &iOut);
		getSpectrum(in1, &rIn1, &iIn1);
		getSpectrum(in2, &rIn2, &iIn2);
		getSpectrum(filter, &rIn3, &iIn3);
		
		if (spectraConform(out, in1, in2, filter, &format, &FFTSize, &beg, &end) == false)
			return false;
		
		if (format == Spectrum::kSpectrumReal)
		{			
			rOut[0] = (rIn1[0] / rIn2[0]) * rIn3[0];
			iOut[0] = (iIn1[0] / iIn2[0]) * iIn3[0];
			
			rOut[0] = std::isinf(rOut[0]) ? 0.0 : rOut[0];
			iOut[0] = std::isinf(iOut[0]) ? 0.0 : iOut[0]; 
		}
		for (uintptr_t i = beg; i < end; i++)
		{
			c1 = std::complex <double> (rIn1[i], iIn1[i]);
			c2 = std::complex <double> (rIn2[i], iIn2[i]);
			c3 = std::complex <double> (rIn3[i], iIn3[i]);
			
			c1 = (c1 * conj(c2)) * c3;
			
			multiplier = 1.0 / norm(c2);
			multiplier = std::isinf(multiplier) ? 0.0 : multiplier;
			
			rOut[i] = real(c1) * multiplier;
			iOut[i] = imag(c1) * multiplier;
		}
		
		return delaySpectrum(out, mDeconvolveDelay);
	}
	
	bool deconvolveWithFilter(Spectrum *in1, Spectrum *in2, Spectrum *filter)
	{
		return deconvolveWithFilter(in1, in1, in2, filter);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////// Deconvolution Filters ///////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
private:
	
	bool changeFilterPhase(Spectrum *filter)
	{
		return changePhase(filter, mDeconvolvePhase, true);
	}

	double regularisationFilter(double real, double beta)
	{
		return 1.0 / (1.0 + (beta / calcPower(real)));
	}
	
	double regularisationFilter(double real, double imag, double beta)
	{
		return 1.0 / (1.0 + (beta / calcPower(real, imag)));
	}
	
    bool transformCustomFilter(Spectrum *filter, Spectrum *denominator, Spectrum::SpectrumFormat *format, uintptr_t *FFTSize, uintptr_t *beg, uintptr_t *end)
    {
        if (spectraConform(filter, denominator, format, FFTSize, beg, end) == false)
            return false;
        
        return timeToSpectrum(mCustomFilter, filter, mCustomFilterLength, *FFTSize, denominator->getSamplingRate());
    }
    
	bool makeRegularisationFilter(Spectrum *denominator, Spectrum *filter)
	{
		double *rOut, *iOut, *rIn1, *iIn1;
		uintptr_t beg, end, FFTSize;		
		Spectrum::SpectrumFormat format;
		
		getSpectrum(denominator, &rIn1, &iIn1);
		getSpectrum(filter, &rOut, &iOut);
		
		if (mCustomFilter == NULL)
		{
			if (spectraConform(filter, denominator, &format, &FFTSize, &beg, &end) == false)
				return false;
			
			if (format == Spectrum::kSpectrumReal)
			{
				rOut[0] = regularisationFilter(rIn1[0], mFilterSpecification.getFirstPowerSpecified(FFTSize, filter->getSamplingRate()));		
				iOut[0] = regularisationFilter(iIn1[0], mFilterSpecification.getNyquistPowerSpecified());
			}
			for (uintptr_t i = beg; i < end; i++)
			{
				rOut[i] = regularisationFilter(rIn1[i], iIn1[i], mFilterSpecification.getNextPowerSpecified());	
				iOut[i] = 0.0;
			}
		}
		else
		{
			if (transformCustomFilter(filter, denominator, &format, &FFTSize, &beg, &end) == false)
				return false;
			         
			if (format == Spectrum::Spectrum::kSpectrumReal)
			{
				rOut[0] = regularisationFilter(rIn1[0], calcPower(rOut[0]));		
				iOut[0] = regularisationFilter(iIn1[0], calcPower(iOut[0]));
			}
			for (uintptr_t i = beg; i < end; i++)
			{
				rOut[i] = regularisationFilter(rIn1[i], iIn1[i], calcPower(rOut[i], iOut[i]));	
				iOut[i] = 0.0;
			}
		}
		
		return changeFilterPhase(filter);
	}
		
	double clipFilter(double real, double minPower, double rangePower)
	{
		double maxPower = minPower * rangePower;
		double divPower = calcPower(real);
		double filterPower = divPower < minPower ? divPower / minPower : 1.0;
		return sqrt(filterPower > maxPower ? divPower / maxPower : filterPower);
	}
	
	double clipFilter(double real, double imag, double minPower, double rangePower)
	
	{
		double maxPower = minPower * rangePower;
		double divPower = calcPower(real, imag);
		double filterPower = divPower < minPower ? divPower / minPower : 1.0;
		return sqrt(filterPower > maxPower ? divPower / maxPower : filterPower);
	}
	
	bool makeClipFilter(Spectrum *denominator, Spectrum *filter)
	{
		double *rOut, *iOut, *rIn1, *iIn1;
		uintptr_t beg, end, FFTSize;		
		Spectrum::SpectrumFormat format;
		
		getSpectrum(denominator, &rIn1, &iIn1);
		getSpectrum(filter, &rOut, &iOut);
		
		if (mCustomFilter == NULL)
		{
			if (spectraConform(filter, denominator, &format, &FFTSize, &beg, &end) == false)
				return false;
			
			if (format == Spectrum::kSpectrumReal)
			{
				rOut[0] = clipFilter(rIn1[0], mFilterSpecification.getFirstPowerSpecified(FFTSize, filter->getSamplingRate()), mRangeSpecification.getFirstPowerSpecified(FFTSize, filter->getSamplingRate()));	
				iOut[0] = clipFilter(iIn1[0], mFilterSpecification.getNyquistPowerSpecified(), mRangeSpecification.getNyquistPowerSpecified());	
			}
			for (uintptr_t i = beg; i < end; i++)
			{
				rOut[i] = clipFilter(rIn1[i], iIn1[i], mFilterSpecification.getNextPowerSpecified(), mRangeSpecification.getNextPowerSpecified());
				iOut[i] = 0;
			}
		}
		else 
		{
            if (transformCustomFilter(filter, denominator, &format, &FFTSize, &beg, &end) == false)
				return false;
			
			if (format == Spectrum::kSpectrumReal)
			{
				rOut[0] = clipFilter(rIn1[0], calcPower(rOut[0]), mRangeSpecification.getFirstPowerSpecified(FFTSize, filter->getSamplingRate()));	
				iOut[0] = clipFilter(iIn1[0], calcPower(iOut[0]), mRangeSpecification.getNyquistPowerSpecified());	
			}
			for (uintptr_t i = beg; i < end; i++)
			{
				rOut[i] = clipFilter(rIn1[i], iIn1[i], calcPower(rOut[i], iOut[i]), mRangeSpecification.getNextPowerSpecified());
				iOut[i] = 0;
			}
		}
		
		return changeFilterPhase(filter);
	}
    
	bool makeFilter(Spectrum *denominator, Spectrum *filter)
	{
		double *rOut, *iOut, *rIn1, *iIn1;
		uintptr_t beg, end, FFTSize;		
		Spectrum::SpectrumFormat format;
		
		getSpectrum(denominator, &rIn1, &iIn1);
		getSpectrum(filter, &rOut, &iOut);
		
		if (mCustomFilter == NULL)
		{
			if (spectraConform(filter, denominator, &format, &FFTSize, &beg, &end) == false)
				return false;
			
			if (format == Spectrum::kSpectrumReal)
			{				
				rOut[0] = sqrt(mFilterSpecification.getFirstPowerSpecified(FFTSize, filter->getSamplingRate()));		
				iOut[0] = sqrt(mFilterSpecification.getNyquistPowerSpecified());	
			}
			for (uintptr_t i = beg; i < end; i++)
			{
				rOut[i] = sqrt(mFilterSpecification.getNextPowerSpecified());	
				iOut[i] = 0.0;
			}
		}
		else
		{
            if (transformCustomFilter(filter, denominator, &format, &FFTSize, &beg, &end) == false)
                return false;
			
			if (mCustomFilterChangePhase == false)
				return true;
		}
		
		return changeFilterPhase(filter);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////// High Level Deconvolution Routines //////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
public:
	
	bool makeDeconvolutionFilter(Spectrum *denominator, Spectrum *filter)
	{		
		switch (mDeconvolveMode)
		{
			case kModeRegularised:
				return makeRegularisationFilter(denominator, filter);
			case kModeClip:
				return makeClipFilter(denominator, filter);
			case kModeFilter:
				return makeFilter(denominator, filter);
		}
		
		return false;
	}
	
    bool deconvolve(Spectrum *out, Spectrum *spectrum1, Spectrum *spectrum2, Spectrum *filter)
	{
		if (makeDeconvolutionFilter(spectrum2, filter) == false)
			return false;
		
		return deconvolveWithFilter(out, spectrum1, spectrum2, filter);
	}
    
	bool deconvolve(Spectrum *spectrum1, Spectrum *spectrum2, Spectrum *filter)
	{
		if (makeDeconvolutionFilter(spectrum2, filter) == false)
			return false;
		
		return deconvolveWithFilter(spectrum1, spectrum2, filter);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////// Analytic Spikes / Delay for Modelling Delays ////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
public:
		
	bool generateSpikeSpectrum (Spectrum *out, uintptr_t FFTSize, Spectrum::SpectrumFormat format, double samplingRate, double spike)
	{
		uintptr_t beg = (out->getFormat() == Spectrum::kSpectrumReal) ? 1: 0;
		uintptr_t end = (out->getFormat() == Spectrum::kSpectrumReal) ? (FFTSize >> 1) : FFTSize;
		
		long double spikeConstant = (long double) (2. * M_PI) * (double) (FFTSize - spike) / ((double) FFTSize);
		long double phase;
		
		double *rIn = out->getSpectrum()->realp;
		double *iIn = out->getSpectrum()->imagp;
		
		if (out->setParams(FFTSize, format, samplingRate) == false)
			return false;
		
		// Do DC + Nyquist
		
		if (format == Spectrum::kSpectrumReal)
		{
			rIn[0] = 1.0;
			iIn[0] = 1.0;
		}
		for (uintptr_t i = beg; i < end; i++)
		{
			phase = spikeConstant * i;
			rIn[i] = cosl(phase);
			iIn[i] = sinl(phase);
		}
        
        return true;
	}
		
	bool delaySpectrum(Spectrum *out, Spectrum *in, double delay)
	{
		double *rOut, *iOut, *rIn1, *iIn1;
		uintptr_t beg, end, FFTSize;		
		Spectrum::SpectrumFormat format;
		
        long double delayConstant;
		long double phase;
	
		getSpectrum(in, &rIn1, &iIn1);
		getSpectrum(out, &rOut, &iOut);
				
		if (delay == 0.0)
		{
			if (in != out)
				return out->copy(in);

			return true;
		}
		
		if (spectraConform(out, in, &format, &FFTSize, &beg, &end) == false)
			return false;
			
		if (format == Spectrum::kSpectrumReal)
		{
			rOut[0] = rIn1[0];
			iOut[0] = iIn1[0];
		}
        
        delayConstant = (long double) (2. * M_PI) * (double) -delay / ((double) FFTSize);
        
		for (uintptr_t i = beg; i < end; i++)
		{
			phase = delayConstant * i;
		
			std::complex <double> c = std::complex <double> (rIn1[i], iIn1[i]) * std::complex <double>(cosl(phase), sinl(phase));
			
			rOut[i] = std::real(c);
			iOut[i] = std::imag(c);
		}
		
		return true;

	}
	
	bool delaySpectrum(Spectrum *in, double delay)
	{
		return delaySpectrum(in, in , delay);
	}
	
	bool timeReverse(Spectrum *out, Spectrum *in)
	{
		double *rOut, *iOut, *rIn1, *iIn1;
		uintptr_t beg, end, FFTSize;		
		Spectrum::SpectrumFormat format;
		
		getSpectrum(in, &rIn1, &iIn1);
		getSpectrum(out, &rOut, &iOut);
		
		if (spectraConform(out, in, &format, &FFTSize, &beg, &end) == false)
				return false;
		
		if (format == Spectrum::kSpectrumReal)
		{
			rOut[0] = rIn1[0];
			iOut[0] = iIn1[0];
		}
		for (uintptr_t i = beg; i < end; i++)
		{
			rOut[i] =  rIn1[i];
			iOut[i] = -iIn1[i];
		}
		
		return true;
	}
	
	bool timeReverse(Spectrum *in)
	{
		return timeReverse(in, in);
	}
};

#endif	/* __HIRT_CORE__ */
