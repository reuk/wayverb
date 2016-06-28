
/*
 *  PartitionedConvolve
 *
 *	PartitionedConvolve performs FFT-based partitioned convolution.
 *	
 *	Typically PartitionedConvolve might be used in conjuction with TimeDomainConvolve for zero-latency convolution with longer impulses.
 *
 *  Copyright 2012 Alex Harker. All rights reserved.
 *
 */

#include "PartitionedConvolve.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __APPLE__
#include <Windows.h>
#endif

// FIX - remove the macro somehow and sort the seeding

// Pointer Utility Macro (cache the offset calculation explictly if we are passing in an expression)

#define DSP_SPLIT_COMPLEX_POINTER_CALC(complex1, complex2, offset)	\
{ \
uintptr_t temp_offset = offset; \
complex1.realp = complex2.realp + temp_offset; \
complex1.imagp = complex2.imagp + temp_offset; \
}

/*
// Random seeding for rand

static __inline unsigned int get_rand_seed ()
{
	unsigned int seed;
	
#ifdef __APPLE__
	seed = arc4random();
#else
	HCRYPTPROV hProvider = 0;
	const DWORD dwLength = 4;
	BYTE *pbBuffer = (BYTE *) &seed;
	
	if (!CryptAcquireContextW(&hProvider, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
		return 0;
	
	CryptGenRandom(hProvider, dwLength, pbBuffer);
	CryptReleaseContext(hProvider, 0);
#endif
	
	return seed;
}


void init_partition_convolve()
{
	srand(get_rand_seed());
}
*/

t_convolve_error HISSTools::PartitionedConvolve::setMaxFFTSize(uintptr_t maxFFTSize)
{
	uintptr_t maxFFTSizeLog2 = log2(maxFFTSize);
	
	t_convolve_error error = CONVOLVE_ERR_NONE;
	
	if (maxFFTSizeLog2 > MAX_FFT_SIZE_LOG2)
	{
		error = CONVOLVE_ERR_FFT_SIZE_MAX_TOO_LARGE;
		maxFFTSizeLog2 = MAX_FFT_SIZE_LOG2;
	}
	
	if (maxFFTSizeLog2 && maxFFTSizeLog2 < MIN_FFT_SIZE_LOG2)
	{
		error = CONVOLVE_ERR_FFT_SIZE_MAX_TOO_SMALL;
		maxFFTSizeLog2 = MIN_FFT_SIZE_LOG2;
	}
	
	if (maxFFTSize != (1 << maxFFTSizeLog2))
		error = CONVOLVE_ERR_FFT_SIZE_MAX_NON_POWER_OF_TWO;
	
    mMaxFFTSizeLog2 = maxFFTSizeLog2;
	mMaxFFTSize = (uintptr_t) 1 << maxFFTSizeLog2;
	
	return error;
}

HISSTools::PartitionedConvolve::PartitionedConvolve(uintptr_t maxFFTSize, uintptr_t maxLength, uintptr_t offset, uintptr_t length)
{
	// Set default initial attributes and variables
	
	mNumPartitions = 0;
	mFFTSizeLog2 = 0;
	mFFTSize = 0;
	
	mMaxImpulseLength = maxLength;
	setMaxFFTSize(maxFFTSize);
	setFFTSize(mMaxFFTSize);
	setOffset(offset);
	setLength(length);
	
	// Allocate impulse buffer and input buffer
		
	uintptr_t maxFFTSizeOver4 = (1 << mMaxFFTSizeLog2) >> 2;
	
	// This is designed to make sure we can load the max impulse length, whatever the fft size
	
	if (mMaxImpulseLength % (maxFFTSizeOver4 << 1))
	{
		mMaxImpulseLength /= (maxFFTSizeOver4 << 1);
		mMaxImpulseLength++;
		mMaxImpulseLength *= (maxFFTSizeOver4 << 1);
	}
	
	mImpulseBuffer.realp = (float *) ALIGNED_MALLOC((mMaxImpulseLength * 4 * sizeof(float)));
	mImpulseBuffer.imagp = mImpulseBuffer.realp + mMaxImpulseLength;
	mInputBuffer.realp = mImpulseBuffer.imagp + mMaxImpulseLength;
	mInputBuffer.imagp = mInputBuffer.realp + mMaxImpulseLength;
	
	// Allocate fft and temporary buffers	
	
	mFFTBuffers[0] = (vFloat *) ALIGNED_MALLOC((maxFFTSizeOver4 * 6 * sizeof(vFloat)));
	mFFTBuffers[1] = mFFTBuffers[0] + maxFFTSizeOver4;
	mFFTBuffers[2] = mFFTBuffers[1] + maxFFTSizeOver4;
	mFFTBuffers[3] = mFFTBuffers[2] + maxFFTSizeOver4;
	
	mAccumBuffer.realp = (float *) (mFFTBuffers[3] + maxFFTSizeOver4);
	mAccumBuffer.imagp = mAccumBuffer.realp + (maxFFTSizeOver4 * 2);
	mPartitionTemp.realp = mAccumBuffer.imagp + (maxFFTSizeOver4 * 2);
	mPartitionTemp.imagp = mPartitionTemp.realp + (maxFFTSizeOver4 * 2);
		
	mFFTSetup = hisstools_create_setup_f(mMaxFFTSizeLog2);
}

HISSTools::PartitionedConvolve::~PartitionedConvolve()
{
    hisstools_destroy_setup_f(mFFTSetup);
    
    // FIX - try to do better here...
    
    ALIGNED_FREE(mImpulseBuffer.realp);
    ALIGNED_FREE(mFFTBuffers[0]);
}

uintptr_t HISSTools::PartitionedConvolve::log2(uintptr_t value)
{
    uintptr_t bitShift = value;
    uintptr_t bitCount = 0;
    
    while (bitShift)
    {
        bitShift >>= 1U;
        bitCount++;
    }
    
    if (value == 1U << (bitCount - 1U))
        return bitCount - 1U;
    else
        return bitCount;
}

t_convolve_error HISSTools::PartitionedConvolve::setFFTSize(uintptr_t FFTSize)
{
	uintptr_t FFTSizeLog2 = log2(FFTSize);
	
	t_convolve_error error = CONVOLVE_ERR_NONE;
	
	if (FFTSizeLog2 < MIN_FFT_SIZE_LOG2 || FFTSizeLog2 > mMaxFFTSizeLog2)
		return CONVOLVE_ERR_FFT_SIZE_OUT_OF_RANGE;
	
	if (FFTSize != (1 << FFTSizeLog2))
		error = CONVOLVE_ERR_FFT_SIZE_NON_POWER_OF_TWO;
	
	// Set fft variables iff the fft size has actually actually changed
	
	if (FFTSizeLog2 != mFFTSizeLog2)
	{
		mNumPartitions = 0;
		
		// Initialise fft info
		
		mFFTSize = (uintptr_t) 1 << FFTSizeLog2;
		mFFTSizeLog2 = FFTSizeLog2;
	}
	
	return error;
}

t_convolve_error HISSTools::PartitionedConvolve::setLength(uintptr_t length)
{
	t_convolve_error error = CONVOLVE_ERR_NONE;
	
	if (length > mMaxImpulseLength)
	{
		error = CONVOLVE_ERR_PARTITION_LENGTH_TOO_LARGE;
		length = mMaxImpulseLength;
	}
	
	mLength = length;
	
	return error;
}

void HISSTools::PartitionedConvolve::setOffset(uintptr_t offset)
{
	mOffset = offset;
}

t_convolve_error HISSTools::PartitionedConvolve::set(const float *input, uintptr_t length)
{	
	t_convolve_error error = CONVOLVE_ERR_NONE;
	
	// FFT variables / attributes
		
	uintptr_t bufferPosition;
	uintptr_t FFTSize = mFFTSize;
	uintptr_t FFTSizeHalved = FFTSize >> 1;
	
	// Partition variables
	
	float *bufferTemp1 = (float *) mPartitionTemp.realp;
	FFT_SPLIT_COMPLEX_F bufferTemp2;
	
	uintptr_t numPartitions;
	
	// Calculate how much of the buffer to load
	
    length = (!input || length <= mOffset) ? 0 : length - mOffset;
    length = (mLength && mLength < length) ? mLength : length;

	if (length > mMaxImpulseLength)
	{
		length = mMaxImpulseLength;
		error = CONVOLVE_ERR_MEM_ALLOC_TOO_SMALL;
	}
	
	// Partition / load the impulse
	
	for (bufferPosition = mOffset, bufferTemp2 = mImpulseBuffer, numPartitions = 0; length > 0; bufferPosition += FFTSizeHalved, numPartitions++)
	{
		// Get samples up to half the fft size
			
		uintptr_t numSamps = (length > FFTSizeHalved) ? FFTSizeHalved : length;
		length -= numSamps;
		
		// Get samples
		
		for (uintptr_t i = 0; i < numSamps; i++)
			bufferTemp1[i] = input[bufferPosition + i];
								 
		// Zero pad
		
		for (uintptr_t i = numSamps; i < FFTSize; i++)
			bufferTemp1[i] = 0;
			
		// Do fft straight into position
		
		hisstools_unzip_f(bufferTemp1, &bufferTemp2, mFFTSizeLog2);
		hisstools_rfft_f(mFFTSetup, &bufferTemp2, mFFTSizeLog2);
		DSP_SPLIT_COMPLEX_POINTER_CALC(bufferTemp2, bufferTemp2, FFTSizeHalved);
	}

    mNumPartitions = numPartitions;
    reset();
	
	return error;
}

void HISSTools::PartitionedConvolve::reset()
{
    mResetFlag = true;
}

bool HISSTools::PartitionedConvolve::process(const float *in, float *out, uintptr_t numSamples)
{		
	FFT_SPLIT_COMPLEX_F impulse_temp, buffer_temp;	
	
	// Scheduling variables
	
	uintptr_t numPartitions = mNumPartitions;
	uintptr_t validPartitions = mValidPartitions;
	uintptr_t partitionsDone = mPartitionsDone;
	uintptr_t lastPartition = mLastPartition;
	uintptr_t nextPartition;
	
	intptr_t numPartitionsToDo;
	
	uintptr_t inputPosition = mInputPosition;
	
	// FFT variables
		
	vFloat *tempVPointer;
	
	uintptr_t FFTSize = mFFTSize;
	uintptr_t FFTSizeHalved = FFTSize >> 1 ;
	uintptr_t FFTSizeHalvedOver4 = FFTSizeHalved >> 2;
	uintptr_t FFTSizeLog2 = mFFTSizeLog2;
	
	uintptr_t RWCounter = mRWCounter;
    uintptr_t hop_mask = FFTSizeHalved - 1;
	
	uintptr_t samplesRemaining = numSamples;
	uintptr_t loopSize;
    uintptr_t hiCounter;

    bool FFTNow;
	
	vFloat vScaleMul = float2vector((float) (1.0 / (double) (FFTSize << 2)));	
	
	if  (!numPartitions)
		return false;
	
	// If we need to reset everything we do that here - happens when the fft size changes, or a new buffer is loaded
	
	if (mResetFlag)
	{
		// Reset fft buffers + accum buffer
		
        memset(mFFTBuffers[0], 0, mMaxFFTSize * 5 * sizeof(float));
        
		// Reset fft RWCounter (randomly)
		
		while (FFTSizeHalved < (uintptr_t) (RWCounter = rand() / (RAND_MAX / FFTSizeHalved)));
				
		// Reset scheduling variables
		
		inputPosition = 0;
		partitionsDone = 0;
		lastPartition = 0;
		validPartitions = 1;
		
		// Set reset flag off
		
		mResetFlag = false;
	}
	
	// Main loop
	
	while (samplesRemaining > 0)
	{
		// Calculate how many IO samples to deal with this loop (depending on whether there is an fft to do before the end of the signal vector)
		
        uintptr_t tillNextFFT = (FFTSizeHalved - (RWCounter & hop_mask));
		loopSize = samplesRemaining < tillNextFFT ? samplesRemaining : tillNextFFT;
        hiCounter = (RWCounter + FFTSizeHalved) & (FFTSize - 1);
        
        // Load input into buffer (twice) and output from the output buffer
        
        memcpy(((float *) mFFTBuffers[0]) + RWCounter, in, loopSize * sizeof(float));

        if ((hiCounter + loopSize) > FFTSize)
        {
            uintptr_t hi_loop = FFTSize - hiCounter;
            memcpy(((float *) mFFTBuffers[1]) + hiCounter, in, hi_loop * sizeof(float));
            memcpy(((float *) mFFTBuffers[1]), in + hi_loop, (loopSize - hi_loop) * sizeof(float));
        }
        else
            memcpy(((float *) mFFTBuffers[1]) + hiCounter, in, loopSize * sizeof(float));

        memcpy(out, ((float *) mFFTBuffers[3]) + RWCounter, loopSize * sizeof(float));

        // Updates to pointers and counters
    
        samplesRemaining -= loopSize;
        RWCounter += loopSize;
        in += loopSize;
        out += loopSize;

        FFTNow = !(RWCounter & hop_mask);
        
		// Work loop and scheduling - this is where most of the convolution is done
		// How many partitions to do this vector? (make sure that all partitions are done before we need to do the next fft)
		     
        if (FFTNow)
            numPartitionsToDo = (validPartitions - partitionsDone) - 1;
        else
            numPartitionsToDo = (((validPartitions - 1) * (RWCounter & hop_mask)) / FFTSizeHalved) - partitionsDone;
            
		while (numPartitionsToDo > 0)
		{
			// Calculate buffer wraparounds (if wraparound is in the middle of this set of partitions this loop will run again)
			
			nextPartition = (lastPartition < numPartitions) ? lastPartition : 0;
			lastPartition = (nextPartition + numPartitionsToDo) > numPartitions ? numPartitions : nextPartition + numPartitionsToDo;
			numPartitionsToDo -= lastPartition - nextPartition;
			
			// Calculate offsets and pointers
			
			DSP_SPLIT_COMPLEX_POINTER_CALC(impulse_temp, mImpulseBuffer, ((partitionsDone + 1) * FFTSizeHalved));
			DSP_SPLIT_COMPLEX_POINTER_CALC(buffer_temp, mInputBuffer, (nextPartition * FFTSizeHalved));
			
			// Do processing
			
			for (uintptr_t i = nextPartition; i < lastPartition; i++)
			{	
				processPartition(buffer_temp, impulse_temp, mAccumBuffer, FFTSizeHalvedOver4);
				DSP_SPLIT_COMPLEX_POINTER_CALC(impulse_temp, impulse_temp, FFTSizeHalved);
				DSP_SPLIT_COMPLEX_POINTER_CALC(buffer_temp, buffer_temp, FFTSizeHalved);
				partitionsDone++;
			}
		}
		
		// FFT processing - this is where we deal with the fft, the first partition and inverse fft
		
		if (FFTNow)
		{			
			// Calculate the position to do the fft from/ to and calculate relevant pointers
			
			tempVPointer = (RWCounter == FFTSize) ? mFFTBuffers[1] : mFFTBuffers[0];
			DSP_SPLIT_COMPLEX_POINTER_CALC(buffer_temp, mInputBuffer, (inputPosition * FFTSizeHalved));
						
			// Do the fft and put into the input buffer
			
			hisstools_unzip_f((float *) tempVPointer, &buffer_temp, FFTSizeLog2);
			hisstools_rfft_f(mFFTSetup, &buffer_temp, FFTSizeLog2);
			
			// Process first partition here and accumulate the output (we need it now!)
			
			processPartition(buffer_temp, mImpulseBuffer, mAccumBuffer, FFTSizeHalvedOver4);
			
			// Processing done - do inverse fft on the accumulation buffer
			
			hisstools_rifft_f(mFFTSetup, &mAccumBuffer, FFTSizeLog2);
			hisstools_zip_f(&mAccumBuffer, (float *) mFFTBuffers[2], FFTSizeLog2);
			
			// Calculate temporary output pointer
			
            tempVPointer = (RWCounter == FFTSize) ? mFFTBuffers[3] : mFFTBuffers[3] + FFTSizeHalvedOver4;
			
			// Scale and store into output buffer (overlap-save) 
			
			for (uintptr_t i = 0; i < FFTSizeHalvedOver4; i++)
				*(tempVPointer++) = F32_VEC_MUL_OP(*(mFFTBuffers[2] + i), vScaleMul);
			
			// Clear accumulation buffer

            memset(mAccumBuffer.realp, 0, FFTSizeHalved * sizeof(float));
            memset(mAccumBuffer.imagp, 0, FFTSizeHalved * sizeof(float));
            
			// Update RWCounter
			
            RWCounter = RWCounter & (FFTSize - 1);
					
			// Set scheduling variables
			            
            validPartitions = (validPartitions == numPartitions) ? validPartitions : validPartitions + 1;
            inputPosition = inputPosition ? inputPosition - 1 : numPartitions - 1;
            lastPartition = inputPosition + 1;
			partitionsDone = 0;
		}
	}
	
	// Write all variables back into the object
	
	mInputPosition = inputPosition;
	mRWCounter = RWCounter;
    
	mValidPartitions = validPartitions;
	mPartitionsDone = partitionsDone;
	mLastPartition = lastPartition;
	
	return true;
}

void HISSTools::PartitionedConvolve::processPartition(FFT_SPLIT_COMPLEX_F in1, FFT_SPLIT_COMPLEX_F in2, FFT_SPLIT_COMPLEX_F out, uintptr_t numVecs)
{
	vFloat *inReal1 = (vFloat *) in1.realp;
	vFloat *inImag1 = (vFloat *) in1.imagp;
	vFloat *inReal2 = (vFloat *) in2.realp;
	vFloat *inImag2 = (vFloat *) in2.imagp;
	vFloat *outReal = (vFloat *) out.realp;
	vFloat *outImag = (vFloat *) out.imagp;
		
	//	Do Nyquist Calculation and then zero these bins
	
	float nyquist1 = in1.imagp[0];
	float nyquist2 = in2.imagp[0];
	
	out.imagp[0] += nyquist1 * nyquist2;

	in1.imagp[0] = 0.f;
	in2.imagp[0] = 0.f;
	
	// Do other bins (loop unrolled)
	
	for (uintptr_t i = 0; i + 3 < numVecs; i += 4)
	{
		outReal[i+0] = F32_VEC_ADD_OP(outReal[i+0], F32_VEC_SUB_OP(F32_VEC_MUL_OP(inReal1[i+0], inReal2[i+0]), F32_VEC_MUL_OP(inImag1[i+0], inImag2[i+0])));
		outImag[i+0] = F32_VEC_ADD_OP(outImag[i+0], F32_VEC_ADD_OP(F32_VEC_MUL_OP(inReal1[i+0], inImag2[i+0]), F32_VEC_MUL_OP(inImag1[i+0], inReal2[i+0])));
		outReal[i+1] = F32_VEC_ADD_OP(outReal[i+1], F32_VEC_SUB_OP(F32_VEC_MUL_OP(inReal1[i+1], inReal2[i+1]), F32_VEC_MUL_OP(inImag1[i+1], inImag2[i+1])));
		outImag[i+1] = F32_VEC_ADD_OP(outImag[i+1], F32_VEC_ADD_OP(F32_VEC_MUL_OP(inReal1[i+1], inImag2[i+1]), F32_VEC_MUL_OP(inImag1[i+1], inReal2[i+1])));
		outReal[i+2] = F32_VEC_ADD_OP(outReal[i+2], F32_VEC_SUB_OP(F32_VEC_MUL_OP(inReal1[i+2], inReal2[i+2]), F32_VEC_MUL_OP(inImag1[i+2], inImag2[i+2])));
		outImag[i+2] = F32_VEC_ADD_OP(outImag[i+2], F32_VEC_ADD_OP(F32_VEC_MUL_OP(inReal1[i+2], inImag2[i+2]), F32_VEC_MUL_OP(inImag1[i+2], inReal2[i+2])));
		outReal[i+3] = F32_VEC_ADD_OP(outReal[i+3], F32_VEC_SUB_OP(F32_VEC_MUL_OP(inReal1[i+3], inReal2[i+3]), F32_VEC_MUL_OP(inImag1[i+3], inImag2[i+3])));
		outImag[i+3] = F32_VEC_ADD_OP(outImag[i+3], F32_VEC_ADD_OP(F32_VEC_MUL_OP(inReal1[i+3], inImag2[i+3]), F32_VEC_MUL_OP(inImag1[i+3], inReal2[i+3])));
	}

	// Replace nyquist bins
	
    in1.imagp[0] = nyquist1;
	in2.imagp[0] = nyquist2;
}
