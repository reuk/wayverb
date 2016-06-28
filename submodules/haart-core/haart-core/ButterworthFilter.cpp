
#include "ButterworthFilter.h"

#include <cmath>

namespace HISSTools
{
    ButterworthFilter::ButterworthFilter(uint32_t maxOrder) : mType(), mOrder(), mCutoff(0.0), mSamplingRate(0.0), mReset(false)
    {
        mBiquads.resize((maxOrder + 1) / 2);
    }
        
    // DSP Engine/Filter Reset
        
    void ButterworthFilter::reset(double samplingRate)
    {
        if (mSamplingRate != samplingRate)
            mReset = true;
        
        mSamplingRate = samplingRate;
        
        switch (mType)
        {
            case kButterworthLPF:
                setLPF(mCutoff, mOrder);
                break;
                
            case kButterworthHPF:
                 setHPF(mCutoff, mOrder);
                break;
        }
        
        for (std::vector<BiquadFilter>::iterator it = mBiquads.begin(); it != mBiquads.end(); it++)
            it->reset();
    }

    // Set filter parameters and type
        
    void ButterworthFilter::setLPF(double cutoff, uint32_t order)
    {
        if (!change(kButterworthLPF, cutoff, order))
            return;
        
        double cf = tan(M_PI * cutoff / mSamplingRate);
        double cf2 = cf * cf;
        
        for (unsigned int i = 0; i < (mOrder / 2); i++)
        {
            double p = 2.0 * cf * cos(M_PI * (mOrder + 2 * (i + 1) - 1) / (2 * mOrder));
            BiquadFilter::Coefficients coeff = BiquadFilter::convertCoefficients(1.0 - p + cf2, 2.0 * (cf2 - 1.0), cf2 + p + 1.0, cf2, 2.0 * cf2, cf2);
            mBiquads[i].set(coeff);
        }
        
        if (mOrder % 2)
        {
            BiquadFilter::Coefficients coeff = BiquadFilter::convertCoefficients(1.0 + cf, cf - 1.0, 0.0, cf, cf, 0.0);
            mBiquads[numBiquads() - 1].set(coeff);
        }
        
    }
    
    void ButterworthFilter::setHPF(double cutoff, uint32_t order)
    {
        if (!change(kButterworthHPF, cutoff, order))
            return;
        
        double cf = tan(M_PI * cutoff / mSamplingRate);
        double cf2 = cf * cf;
        double cf3 = cf2 * cf;
        
        for (unsigned int i = 0; i < (mOrder / 2); i++)
        {
            double p = 2.0 * cf2 * cos(M_PI * (mOrder + 2 * (i + 1) - 1) / (2 * mOrder));
            BiquadFilter::Coefficients coeff = BiquadFilter::convertCoefficients(cf - p + cf3, 2.0 * (cf3 - cf), cf3 + p + cf, cf, -2.0 * cf, cf);
            mBiquads[i].set(coeff);
        }
        
        if (mOrder % 2)
        {
            BiquadFilter::Coefficients coeff = BiquadFilter::convertCoefficients(1.0 + cf, cf - 1.0, 0.0, 1.0, -1.0, 0.0);
            mBiquads[numBiquads() - 1].set(coeff);
        }
        
    }
    
    // DSP
        
    double ButterworthFilter::operator()(double input)
    {
        for (uint32_t i = 0; i < numBiquads(); i++)
            input = mBiquads[i](input);
        
        return input;
    }

    void ButterworthFilter::process(double* io, uintptr_t numSamples)
    {
        for (uintptr_t i = 0; i < numSamples; i++)
            io[i] = ButterworthFilter::operator()(io[i]);
    }
    
    void ButterworthFilter::process(const double* input, double *output, uintptr_t numSamples)
    {
        for (uintptr_t i = 0; i < numSamples; i++)
            output[i] = ButterworthFilter::operator()(input[i]);
    }
        
    void ButterworthFilter::reverseProcess(double* io, uintptr_t numSamples)
    {
        for (uintptr_t i = numSamples; i > 0; i--)
            io[i - 1] = ButterworthFilter::operator()(io[i - 1]);
    }
    
    void ButterworthFilter::reverseProcess(const double* input, double *output, uintptr_t numSamples)
    {
        for (uintptr_t i = numSamples; i > 0; i--)
            output[i] = ButterworthFilter::operator()(input[i]);
    }
    
    bool ButterworthFilter::change(FilterType type, double cutoff, uint32_t order)
    {
        uint32_t maxOrder = static_cast<uint32_t>(mBiquads.size() * 2);
        
        order = order > maxOrder ? maxOrder : order;
        
        if (!mReset && type == mType && order == mOrder && cutoff == mCutoff)
            return false;
        
        mOrder = order;
        mCutoff = cutoff;
        
        return true;
    }
    
    uint32_t ButterworthFilter::numBiquads()
    {
        return (mOrder + 1) / 2;
    }
}
