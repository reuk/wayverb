

#ifndef __ZEROLATENCYCONVOLVE__
#define __ZEROLATENCYCONVOLVE__

#include "PartitionedConvolve.h"
#include "TimeDomainConvolve.h"
#include "convolve_errors.h"

#include "AH_Generic_Memory_Swap.h"

#include <stdint.h>

typedef enum
{
    CONVOLVE_LATENCY_ZERO = 0,
    CONVOLVE_LATENCY_SHORT = 1,
    CONVOLVE_LATENCY_MEDIUM = 2,
    
} t_convolve_latency_mode;

namespace HISSTools
{
    class MonoConvolve
    {
        
    public:
        
        MonoConvolve(uintptr_t maxLength, t_convolve_latency_mode latency);
        ~MonoConvolve();
        
        PartitionedConvolve *resize(uintptr_t length, bool keepLock);
        t_convolve_error set(const float *input, uintptr_t length, bool resizeFlag);
        t_convolve_error reset();
        
        void sum(float *out, float *add, uintptr_t numSamples);
        void process(const float *in, float *temp, float *out, uintptr_t numSamples);
        
    private:
        
        TimeDomainConvolve *mTime1;
        PartitionedConvolve *mPart1;
        PartitionedConvolve *mPart2;
        PartitionedConvolve *mPart3;
        
        t_memory_swap mPart4;
        
        uintptr_t mLength;
        t_convolve_latency_mode mLatency;
    };
}

#endif // __ZEROLATENCYCONVOLVE__

