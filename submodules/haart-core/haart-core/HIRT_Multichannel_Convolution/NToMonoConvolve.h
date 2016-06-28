
#ifndef __OUTPUTCHANNELCONVOLVER__
#define __OUTPUTCHANNELCONVOLVER__
	
#include "MonoConvolve.h"
#include "convolve_errors.h"

#include <stdint.h>
#include <vector>

namespace HISSTools
{
    class NToMonoConvolve
    {
        
    public:
        
        NToMonoConvolve(uint32_t input_chans, uintptr_t maxLength, t_convolve_latency_mode latency);
        ~NToMonoConvolve();
        
        t_convolve_error resize(uint32_t inChan, uintptr_t impulse_length);
        t_convolve_error set(uint32_t inChan, const float *input, uintptr_t impulse_length, bool resize);
        t_convolve_error reset(uint32_t inChan);
        
        void process(const float **ins, float *out, float *temp1, size_t numSamples, size_t active_in_chans);
        void process(const float **ins, double *out, float *temp1, float *temp2, size_t numSamples, size_t active_in_chans);
        
    private:
        
        std::vector<MonoConvolve *> mConvolvers;
        
        uint32_t mNumInChans;
    };
}

#endif