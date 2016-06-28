
#ifndef _HISSTOOLS_EXCITATIONSIGNALS_
#define _HISSTOOLS_EXCITATIONSIGNALS_

#include <stdint.h>
#include <vector>

#include "FrequencyCurve.h"

namespace HISSTools
{
    struct Excitation
    {
        Excitation(double IRLength, double samplingRate, uint32_t numIns, uint32_t numOuts, double amp)
        : mPosition(0), mIRLength(round(IRLength * samplingRate)), mNumIns(numIns), mNumOuts(numOuts), mAmp(amp), mSamplingRate(samplingRate) {}
        virtual ~Excitation() {}
            
        // Clone
            
        virtual Excitation *clone() const = 0;
            
        // Length and harmonic offset
            
        uintptr_t getMeasurementLength()       const                { return (mSignalLength + mIRLength) * mNumOuts; }
        uintptr_t getSignalLength()            const                { return mSignalLength; }
        uintptr_t getIRLength()                const                { return mIRLength; }
        virtual uintptr_t getHarmonicOffset(uint32_t /*harmonic*/)  { return 0; }
            
        // Support for hamonic separation and use of inverse signal
        
        virtual bool hasInverse()               { return false; }
        virtual bool separatesHarmonics()       { return false; }
            
        // Getters
            
        uint32_t getNumIns()        const       { return mNumIns; }
        uint32_t getNumOuts()       const       { return mNumOuts; }
        uintptr_t getPosition()     const       { return mPosition; }
        double getSamplingRate()    const       { return mSamplingRate; }
        double getNormPosition()    const       { return static_cast<double>(mPosition) / static_cast<double>(getMeasurementLength()); }
        double getAmp()             const       { return mAmp; }
            
        // Reset position of excitation signal
            
        void reset()
        {
            mPosition = 0U;
            internalReset();
        }
            
        // DSP
            
        void generate(double *output, bool useAmp);
        void generateBlock(double *const *const outs, uintptr_t numOuts, uintptr_t numSamples);
            
        virtual void generateInverse(double */*output*/, bool /*useAmp*/) {}
            
    private:
            
        virtual void internalReset() = 0;
        virtual void generateN(double *output, bool useAmp, uintptr_t nSamps) = 0;
            
        // Excitatation Parameters
            
        uintptr_t mPosition;
        uintptr_t mIRLength;
        
        uint32_t mNumIns;
        uint32_t mNumOuts;
            
        double  mAmp;
        double mSamplingRate;
        
    protected:
        
        uintptr_t mSignalLength;
    };
    
    // Exponential Sine Sweep (ESS)
        
    struct ExponentialSineSweep : public Excitation
    {
        ExponentialSineSweep(uint32_t numIns, uint32_t numOuts, double freqLo, double freqHi, double fadeIn, double fadeOut, double length, double IRLength, double samplingRate, double amp, const FrequencyCurve& ampCurve);
        
        // Clone
        
        virtual Excitation *clone() const { return new ExponentialSineSweep(*this); }
        
        // Length and harmonic offset
        
        virtual uintptr_t getHarmonicOffset(uint32_t harmonic);
        
        // Support for hamonic separation and use of inverse signal
        
        virtual bool hasInverse() { return true; }
        virtual bool separatesHarmonics() { return true; }
        
        // DSP
        
        virtual void generateN(double *output, bool useAmp, uintptr_t nSamps);
        virtual void generateInverse(double *output, bool useAmp);
        
    private:
        
        struct freqTimeTransform
        {
            freqTimeTransform(double f1, double samplingRate) : mF1(f1) , mSamplingRate(samplingRate)
            {}
            
            FrequencySpecifier operator()(const FrequencySpecifier &i) const
            {
                return FrequencySpecifier(log(i.mFreq / (mF1 * mSamplingRate)), i.mDB);
            }
            
            double mF1;
            double mSamplingRate;
        };
        
        // Reset internal values of excitation signal
        
        virtual void internalReset();
        
        uintptr_t mPosition;
        
        double mF1;
        double mK1;
        double mK2;
        double mFreqRatio;
        double mFiN;
        double mFoN;
        
        FrequencyCurve mAmpCurve;
    };
    
    // Maximum Length Sequence (MLS)
    
    struct MaximumLengthSequence : public Excitation
    {
        MaximumLengthSequence(uint32_t numIns, uint32_t numOuts, uint32_t order, double IRLength, double samplingRate, double amp);
        
        // Clone
        
        virtual Excitation *clone() const { return new MaximumLengthSequence(*this); }
        
        // DSP
        
        virtual void generateN(double *output, bool useAmp, uintptr_t nSamps);
        
    private:
        
        // Reset internal values of excitation signal
        
        virtual void internalReset();
        
        static const uint32_t MaskValues[];
        
        uint32_t mOrder;
        uint32_t mLSFR;
    };
    
    // Pink Noise
    
    struct PinkNoiseExcitation : public Excitation
    {
        
    private:
        
        // Simple One Pole Filter
        
        struct OnePole
        {
            OnePole() : alpha(0.0), y1(0.0) {};
            OnePole(double freq, double samplingRate);
            void reset();
            double operator()(double x);
            
        private:
            
            double alpha;
            double y1;
        };
        
        // 32 Bit Random Number Generator
        
        struct Random
        {
            Random() { reset(); }
            void reset();
            double operator()();
            
        private:
            
            uint32_t w, x, y, z;
        };
        
    public:
        
        PinkNoiseExcitation(uint32_t numIns, uint32_t numOuts, double fadeIn, double fadeOut, double length, double IRLength, double samplingRate, double amp);
        
        // Clone
        
        virtual Excitation *clone() const { return new PinkNoiseExcitation(*this); }
        
        // DSP
        
        virtual void generateN(double *output, bool useAmp, uintptr_t nSamps);
        
    private:
        
        // Reset internal values of excitation signal
        
        virtual void internalReset();
        
        double mFiN;
        double mFoN;
        double mYa1;
        double mYb1;
        
        uintptr_t mPosition;
        
        Random mRandom;
        std::vector<OnePole> mOnePoles;
    };
}

#endif
