
#ifndef __PARTIONEDCONVOLVE__
#define __PARTIONEDCONVOLVE__


#include "convolve_errors.h"

#include <AH_Headers/AH_VectorOps.h>
#include "../HISSTools_FFT/HISSTools_FFT.h"

#include <stdint.h>

// N.B. MIN_FFT_SIZE_LOG2 should never be smaller than 4, as below code assumes loop unroll of vectors (4 vals) by 4 (== 16 or 2^4)
//		MAX_FFT_SIZE_LOG2 is perhaps conservative right now, assuming realtime usage, but it is easy to increase this if necessary

#define MIN_FFT_SIZE_LOG2					5
#define MAX_FFT_SIZE_LOG2					20


namespace HISSTools
{
    namespace DSP
    {
        class PartitionedConvolve
        {
            
        public:
            
            PartitionedConvolve(uintptr_t maxFFTSize, uintptr_t maxLength, uintptr_t offset, uintptr_t length);
            ~PartitionedConvolve();
            
            t_convolve_error setFFTSize(uintptr_t FFTSize);
            t_convolve_error setLength(uintptr_t length);
            void setOffset(uintptr_t offset);
            
            t_convolve_error set(const float *input, uintptr_t length);
            void reset();
            
            bool process(float *in, float *out, uintptr_t numSamples);

        private:
            
            void processPartition(FFT_SPLIT_COMPLEX_F in1, FFT_SPLIT_COMPLEX_F in2, FFT_SPLIT_COMPLEX_F out, uintptr_t numVecs);
            t_convolve_error setMaxFFTSize(uintptr_t max_fft_size);
            uintptr_t log2(uintptr_t value);

            // FFT variables
            
            FFT_SETUP_F mFFTSetup;
            
            uintptr_t mMaxFFTSize;
            uintptr_t mMaxFFTSizeLog2;
            uintptr_t mFFTSize;
            uintptr_t mFFTSizeLog2;
            
            uintptr_t mRWCounter;
            
            // Scheduling variables
            
            uintptr_t mInputPosition;
            
            uintptr_t mPartitionsDone;
            uintptr_t mLastPartition;
            
            uintptr_t mNumPartitions;
            uintptr_t mValidPartitions;
            
            // Internal buffers
            
            vFloat *fft_buffers[4];
            
            FFT_SPLIT_COMPLEX_F impulse_buffer;
            FFT_SPLIT_COMPLEX_F	input_buffer;
            FFT_SPLIT_COMPLEX_F	accum_buffer;
            FFT_SPLIT_COMPLEX_F	partition_temp;
            
            uintptr_t mMaxImpulseLength;
            
            // Attributes
            
            uintptr_t mOffset;
            uintptr_t mLength;
            
            // Flags
            
            bool mResetFlag;
        };
    }
}

#endif // __PARTIONEDCONVOLVE__
