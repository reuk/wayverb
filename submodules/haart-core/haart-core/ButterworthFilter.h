
#ifndef _HISSTOOLS_BUTTERWORTHFILTER_
#define _HISSTOOLS_BUTTERWORTHFILTER_

#include <stdint.h>
#include <vector>

#include "BiquadFilter.h"

namespace HISSTools
{
    class ButterworthFilter
    {

    public:
        
        enum FilterType { kButterworthLPF, kButterworthHPF };
        
        ButterworthFilter(uint32_t maxOrder = 8);
        
        // DSP Engine/Filter Reset
        
        void reset(double samplingRate);

        // Set filter parameters and type
        
        void setLPF(double cutoff, uint32_t order);
        void setHPF(double cutoff, uint32_t order);
        void setBPF(double loCutoff, double hiCutoff, uint32_t order);

        // DSP
        
        double operator()(double input);

        void process(double* io, uintptr_t numSamples);
        void process(const double* input, double *output, uintptr_t numSamples);
        
        void reverseProcess(double* io, uintptr_t numSamples);
        void reverseProcess(const double* input, double *output, uintptr_t numSamples);

    private:
        
        bool change(FilterType type, double cutoff, uint32_t order);
        uint32_t numBiquads();
        
        // Data
        
        std::vector<BiquadFilter> mBiquads;
        
        FilterType mType;
        uint32_t mOrder;
        double mCutoff;
        double mSamplingRate;
        bool mReset;
    };
}

#endif