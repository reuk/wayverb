
#ifndef __TIMEDOMAINCONVOLVE__
#define __TIMEDOMAINCONVOLVE__

#include "convolve_errors.h"

#include <stdint.h>


namespace HISSTools
{
    namespace DSP
    {
        class TimeDomainConvolve
        {
     
        public:
            
            TimeDomainConvolve(uintptr_t offset, uintptr_t length);
            ~TimeDomainConvolve();
            
            t_convolve_error setLength(uintptr_t length);
            void setOffset(uintptr_t offset);
            
            t_convolve_error set(const float *input, uintptr_t length);
            void reset();
            
            void process(float *in, float *out, uintptr_t numSamples);

        private:
            
            void processScalar(float *in, float *out, uintptr_t numSamples);
            void processSIMD(float *in, float *out, uintptr_t numSamples);
            
            // Internal buffers
            
            float *mImpulseBuffer;
            float *mInputBuffer;
            
            uintptr_t mInputPosition;
            uintptr_t mImpulseLength;
            
            uintptr_t mOffset;
            uintptr_t mLength;
            
            // Flags
            
            bool mReset;
        };
    }
}

#endif /* __TIMEDOMAINCONVOLVE__ */
