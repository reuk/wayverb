
/*
 *  TimeDomainConvolve
 *
 *	TimeDomainConvolve performs real-time zero latency time-based convolution.
 *	
 *	Typically TimeDomainConvolve is suitable for use in conjunction with PartitionedConvolve for zero-latency convolution with longer impulses (time_domain_convolve use apple's vDSP and the IR length is limited to 2044 samples).
 *	Note that in fact the algorithms process correlation with reversed impulse response coeffients - which is equivalent to convolution.
 *
 *  Copyright 2012 Alex Harker. All rights reserved.
 *
 */

#include "TimeDomainConvolve.h"

#include "AH_VectorOps.h"

#include <stdio.h>
#include <string.h>


uintptr_t pad_length(uintptr_t length)
{
	return ((length + 15) >> 4) << 4;
}

HISSTools::TimeDomainConvolve::TimeDomainConvolve(uintptr_t offset, uintptr_t length) : mInputPosition(0), mImpulseLength(0)
{
	// Set default initial variables

	setOffset(offset);
	setLength(length);
	
	// Allocate impulse buffer and input bufferr
	
	mImpulseBuffer = (float *) ALIGNED_MALLOC (sizeof(float) * 2048);
	mInputBuffer = (float *) ALIGNED_MALLOC (sizeof(float) * 8192);
		
    // Zero buffers
    
    memset(mImpulseBuffer, 0, 2048 * sizeof(float));
    memset(mInputBuffer, 0, 8192 * sizeof(float));
}

HISSTools::TimeDomainConvolve::~TimeDomainConvolve()
{
    ALIGNED_FREE(mImpulseBuffer);
    ALIGNED_FREE(mInputBuffer);
}

void HISSTools::TimeDomainConvolve::setOffset(uintptr_t offset)
{
	mOffset = offset;
}

t_convolve_error HISSTools::TimeDomainConvolve::setLength(uintptr_t length)
{	
	t_convolve_error error = CONVOLVE_ERR_NONE;

	if (length > 2044)
	{
		error = CONVOLVE_ERR_TIME_LENGTH_OUT_OF_RANGE;
		length = 2044;
	}
	
	mLength = length;
	
	return error;
}

t_convolve_error HISSTools::TimeDomainConvolve::set(const float *input, uintptr_t length)
{
	t_convolve_error error = CONVOLVE_ERR_NONE;
	
	mImpulseLength = 0;
	
	// Calculate impulse length
	
	if (!input || length < mOffset)
		mImpulseLength = 0;
	
	length -= mOffset;
	if (mLength && mLength < length)
		mImpulseLength = mLength;
	
	if (length > 2044)
	{
		error = CONVOLVE_ERR_TIME_IMPULSE_TOO_LONG;
		mImpulseLength = 2044;
	}
		
#ifdef __APPLE__
	if (mImpulseLength)
	{
		for (uintptr_t i = mImpulseLength, j = 0; i > 0; i--, j++)
			mImpulseBuffer[j] = input[i + mOffset - 1];
	}
#else
	if (mImpulseLength)
	{
		uintptr_t impulse_offset = pad_length(mImpulseLength) - mImpulseLength;

		for (uintptr_t i = 0; i < impulse_offset; i++)
			mImpulseBuffer[i] = 0.f;

		for (uintptr_t i = mImpulseLength, j = 0; i > 0; i--, j++)
			mImpulseBuffer[j + impulse_offset] = input[i + mOffset - 1];
	}
#endif
		    
    mReset = true;
	
	return error;
}

void HISSTools::TimeDomainConvolve::reset()
{
    mReset = true;
}

#ifndef __APPLE__

void convolve(const float *in, vFloat *impulse, float *output, uintptr_t N, uintptr_t L)
{
	vFloat output_accum;
	float *input;
	float results[4];
		
	L = pad_length(L);
				   
	for (uintptr_t i = 0; i < N; i++)
	{
		output_accum = float2vector(0.f);
		input = in - L + 1 + i;
		
		for (uintptr_t j = 0; j < L >> 2; j += 4)
		{
			// Load vals
			
			output_accum = F32_VEC_ADD_OP(output_accum, F32_VEC_MUL_OP(impulse[j], F32_VEC_ULOAD(input)));
			input += 4;
			output_accum = F32_VEC_ADD_OP(output_accum, F32_VEC_MUL_OP(impulse[j + 1], F32_VEC_ULOAD(input)));
			input += 4;
			output_accum = F32_VEC_ADD_OP(output_accum, F32_VEC_MUL_OP(impulse[j + 2], F32_VEC_ULOAD(input)));
			input += 4;
			output_accum = F32_VEC_ADD_OP(output_accum, F32_VEC_MUL_OP(impulse[j + 3], F32_VEC_ULOAD(input)));
			input += 4;
		}
		
		F32_VEC_USTORE(results, output_accum);
		
		*output++ = results[0] + results[1] + results[2] + results[3];
	}
}

#endif

void convolve(const float *in, float *impulse, float *output, uintptr_t N, uintptr_t L)
{
	float output_accum;
	const float *input;
		
	L = pad_length(L);
	
	for (uintptr_t i = 0; i < N; i++)
	{
		output_accum = 0.f;
		input = in - L + 1 + i;
		
		for (uintptr_t j = 0; j < L; j += 8)
		{
			// Load vals
			
			output_accum += impulse[j+0] * *input++;
			output_accum += impulse[j+1] * *input++;
			output_accum += impulse[j+2] * *input++;
			output_accum += impulse[j+3] * *input++;
			output_accum += impulse[j+4] * *input++;
			output_accum += impulse[j+5] * *input++;
			output_accum += impulse[j+6] * *input++;
			output_accum += impulse[j+7] * *input++;
		}
		
		*output++ = output_accum;
	}
}

void HISSTools::TimeDomainConvolve::process(const float *in, float *out, uintptr_t numSamples)
{
    if (mReset)
    {
        memset(mInputBuffer, 0, 8192 * sizeof(float));
        mReset = false;
    }
    
    if (numSamples % 4)
        processScalar(in, out, numSamples);
    else
        processSIMD(in, out, numSamples);
}


void HISSTools::TimeDomainConvolve::processScalar(const float *in, float *out, uintptr_t numSamples)
{
	uintptr_t currentLoop;
    
    while ((currentLoop = (mInputPosition + numSamples) > 4096 ? (4096 - mInputPosition) : ((numSamples > 2048) ? 2048 : numSamples)))
	{
		// Copy input twice (allows us to read input out in one go)
	
		memcpy(mInputBuffer + mInputPosition, in, sizeof(float) * currentLoop);
		memcpy(mInputBuffer + 4096 + mInputPosition, in, sizeof(float) * currentLoop);
	
		// Advance pointer 
	
		mInputPosition += currentLoop;
		if (mInputPosition >= 4096)
			mInputPosition -= 4096;
	
		// Do convolution
	
		convolve(mInputBuffer + 4096 + (mInputPosition - currentLoop), mImpulseBuffer, out, currentLoop, mImpulseLength);
        
        // Updates
        
        in += currentLoop;
        out += currentLoop;
        numSamples -= currentLoop;
	}
}

void HISSTools::TimeDomainConvolve::processSIMD(const float *in, float *out, uintptr_t numSamples)
{
	uintptr_t currentLoop;
    
    while ((currentLoop = (mInputPosition + numSamples) > 4096 ? (4096 - mInputPosition) : ((numSamples > 2048) ? 2048 : numSamples)))
	{
		// Copy input twice (allows us to read input out in one go)
		
		memcpy(mInputBuffer + mInputPosition, in, sizeof(float) * currentLoop);
		memcpy(mInputBuffer + 4096 + mInputPosition, in, sizeof(float) * currentLoop);
		
		// Advance pointer 
		
		mInputPosition += currentLoop;
		if (mInputPosition >= 4096)
			mInputPosition -= 4096;
		
		// Do convolution
		
	#ifdef __APPLE__
		vDSP_conv(mInputBuffer + 4096 + mInputPosition - (mImpulseLength + currentLoop) + 1, (vDSP_Stride) 1, mImpulseBuffer, (vDSP_Stride) 1, out, (vDSP_Stride) 1, (vDSP_Length) currentLoop, mImpulseLength);
	#else
		convolve(mInputBuffer + 4096 + (input_position - currentLoop), (vFloat *) mImpulseBuffer, out, currentLoop, mImpulseLength);
	#endif
        
        // Updates
        
        in += currentLoop;
        out += currentLoop;
        numSamples -= currentLoop;
	}
}