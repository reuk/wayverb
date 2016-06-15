
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

t_convolve_error HISSTools::DSP::PartitionedConvolve::setMaxFFTSize(uintptr_t max_fft_size)
{
	uintptr_t maxFFTSizeLog2 = log2(max_fft_size);
	
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
	
	if (max_fft_size != (1 << maxFFTSizeLog2))
		error = CONVOLVE_ERR_FFT_SIZE_MAX_NON_POWER_OF_TWO;
	
    mMaxFFTSizeLog2 = maxFFTSizeLog2;
	mFFTSize = (uintptr_t) 1 << maxFFTSizeLog2;
	
    if (max_fft_size != (1 << maxFFTSizeLog2))
        error = CONVOLVE_ERR_FFT_SIZE_MAX_NON_POWER_OF_TWO;
    
	return error;
}

HISSTools::DSP::PartitionedConvolve::PartitionedConvolve(uintptr_t maxFFTSize, uintptr_t maxLength, uintptr_t offset, uintptr_t length)
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
		
	uintptr_t max_fft_over_4 = (1 << mMaxFFTSizeLog2) >> 2;
	
	// This is designed to make sure we can load the max impulse length, whatever the fft size
	
	if (mMaxImpulseLength % (max_fft_over_4 << 1))
	{
		mMaxImpulseLength /= (max_fft_over_4 << 1);
		mMaxImpulseLength++;
		mMaxImpulseLength *= (max_fft_over_4 << 1);
	}
	
	impulse_buffer.realp = (float *) ALIGNED_MALLOC((mMaxImpulseLength * 4 * sizeof(float)));
	impulse_buffer.imagp = impulse_buffer.realp + mMaxImpulseLength;
	input_buffer.realp = impulse_buffer.imagp + mMaxImpulseLength;
	input_buffer.imagp = input_buffer.realp + mMaxImpulseLength;
	
	// Allocate fft and temporary buffers	
	
	fft_buffers[0] = (vFloat *) ALIGNED_MALLOC((max_fft_over_4 * 6 * sizeof(vFloat)));
	fft_buffers[1] = fft_buffers[0] + max_fft_over_4;
	fft_buffers[2] = fft_buffers[1] + max_fft_over_4;
	fft_buffers[3] = fft_buffers[2] + max_fft_over_4;
	
	accum_buffer.realp = (float *) (fft_buffers[3] + max_fft_over_4);
	accum_buffer.imagp = accum_buffer.realp + (max_fft_over_4 * 2);
	partition_temp.realp = accum_buffer.imagp + (max_fft_over_4 * 2);
	partition_temp.imagp = partition_temp.realp + (max_fft_over_4 * 2);
		
	mFFTSetup = hisstools_create_setup_f(mMaxFFTSizeLog2);
}

HISSTools::DSP::PartitionedConvolve::~PartitionedConvolve()
{
    hisstools_destroy_setup_f(mFFTSetup);
    
    // FIX - try to do better here...
    
    ALIGNED_FREE(impulse_buffer.realp);
    ALIGNED_FREE(fft_buffers[0]);
}

uintptr_t HISSTools::DSP::PartitionedConvolve::log2(uintptr_t value)
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

t_convolve_error HISSTools::DSP::PartitionedConvolve::setFFTSize(uintptr_t FFTSize)
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

t_convolve_error HISSTools::DSP::PartitionedConvolve::setLength(uintptr_t length)
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

void HISSTools::DSP::PartitionedConvolve::setOffset(uintptr_t offset)
{
	mOffset = offset;
}

t_convolve_error HISSTools::DSP::PartitionedConvolve::set(const float *input, uintptr_t length)
{	
	t_convolve_error error = CONVOLVE_ERR_NONE;
	
	// FFT variables / attributes
		
	uintptr_t buffer_pos;
	uintptr_t fft_size = mFFTSize;
	uintptr_t fft_size_halved = fft_size >> 1;
	
	// Partition variables
	
	float *buffer_temp1 = (float *) partition_temp.realp;
	FFT_SPLIT_COMPLEX_F impulse_buffer = impulse_buffer;
	FFT_SPLIT_COMPLEX_F buffer_temp2;
	
	uintptr_t num_partitions;
	
	// Calculate how much of the buffer to load
	
    length = (!input || length <= mOffset) ? 0 : length - mOffset;
    length = (mLength && mLength < length) ? mLength : length;

	if (length > mMaxImpulseLength)
	{
		length = mMaxImpulseLength;
		error = CONVOLVE_ERR_MEM_ALLOC_TOO_SMALL;
	}
	
	// Partition / load the impulse
	
	for (buffer_pos = mOffset, buffer_temp2 = impulse_buffer, num_partitions = 0; length > 0; buffer_pos += fft_size_halved, num_partitions++)
	{
		// Get samples up to half the fft size
			
		uintptr_t numSamps = (length > fft_size_halved) ? fft_size_halved : length;
		length -= numSamps;
		
		// Get samples
		
		for (uintptr_t i = 0; i < numSamps; i++)
			buffer_temp1[i] = input[buffer_pos + i];
								 
		// Zero pad
		
		for (uintptr_t i = numSamps; i < fft_size; i++)
			buffer_temp1[i] = 0;
			
		// Do fft straight into position
		
		hisstools_unzip_f(buffer_temp1, &buffer_temp2, mFFTSizeLog2);
		hisstools_rfft_f(mFFTSetup, &buffer_temp2, mFFTSizeLog2);
		DSP_SPLIT_COMPLEX_POINTER_CALC(buffer_temp2, buffer_temp2, fft_size_halved);
	}

    mNumPartitions = num_partitions;
    reset();
	
	return error;
}

void HISSTools::DSP::PartitionedConvolve::reset()
{
    mResetFlag = true;
}

bool HISSTools::DSP::PartitionedConvolve::process(float *in, float *out, uintptr_t numSamples)
{		
	FFT_SPLIT_COMPLEX_F impulse_buffer = impulse_buffer;
	FFT_SPLIT_COMPLEX_F input_buffer = input_buffer;
	FFT_SPLIT_COMPLEX_F accum_buffer = accum_buffer;
	FFT_SPLIT_COMPLEX_F impulse_temp, buffer_temp;	
	
	// Scheduling variables
	
	uintptr_t num_partitions = mNumPartitions;
	uintptr_t valid_partitions = mValidPartitions;
	uintptr_t partitions_done = mPartitionsDone;
	uintptr_t last_partition = mLastPartition;
	uintptr_t next_partition;
	
	intptr_t num_partitions_to_do;
	
	uintptr_t input_position = mInputPosition;
	
	// FFT variables
		
	vFloat **fft_buffers = fft_buffers;
	vFloat *temp_vpointer;
	
	uintptr_t fft_size = mFFTSize;
	uintptr_t fft_size_halved = fft_size >> 1 ;
	uintptr_t fft_size_halved_over_4 = fft_size_halved >> 2;
	uintptr_t fft_size_log2 = mFFTSizeLog2;
	
	uintptr_t rw_counter = mRWCounter;
    uintptr_t hop_mask = fft_size_halved - 1;
	
	uintptr_t vec_remain = numSamples;
	uintptr_t loop_size;
    uintptr_t hi_counter;

    bool fft_now;
	
	vFloat vscale_mult = float2vector((float) (1.0 / (double) (fft_size << 2)));	
	
	if  (!num_partitions)
		return false;
	
	// If we need to reset everything we do that here - happens when the fft size changes, or a new buffer is loaded
	
	if (mResetFlag)
	{
		// Reset fft buffers + accum buffer
		
        memset(fft_buffers[0], 0, mMaxFFTSize * 5 * sizeof(float));
        
		// Reset fft rw_counter (randomly)
		
		while (fft_size_halved < (uintptr_t) (rw_counter = rand() / (RAND_MAX / fft_size_halved)));
				
		// Reset scheduling variables
		
		input_position = 0;
		partitions_done = 0;
		last_partition = 0;
		valid_partitions = 1;
		
		// Set reset flag off
		
		mResetFlag = false;
	}
	
	// Main loop
	
	while (vec_remain > 0)
	{
		// Calculate how many IO samples to deal with this loop (depending on whether there is an fft to do before the end of the signal vector)
		
        uintptr_t till_next_fft = (fft_size_halved - (rw_counter & hop_mask));
		loop_size = vec_remain < till_next_fft ? vec_remain : till_next_fft;
        hi_counter = (rw_counter + fft_size_halved) & (fft_size - 1);
        
        // Load input into buffer (twice) and output from the output buffer
        
        memcpy(((float *) fft_buffers[0]) + rw_counter, in, loop_size * sizeof(float));

        if ((hi_counter + loop_size) > fft_size)
        {
            uintptr_t hi_loop = fft_size - hi_counter;
            memcpy(((float *) fft_buffers[1]) + hi_counter, in, hi_loop * sizeof(float));
            memcpy(((float *) fft_buffers[1]), in + hi_loop, (loop_size - hi_loop) * sizeof(float));
        }
        else
            memcpy(((float *) fft_buffers[1]) + hi_counter, in, loop_size * sizeof(float));

        memcpy(out, ((float *) fft_buffers[3]) + rw_counter, loop_size * sizeof(float));

        // Updates to pointers and counters
    
        vec_remain -= loop_size;
        rw_counter += loop_size;
        in += loop_size;
        out += loop_size;

        fft_now = !(rw_counter & hop_mask);
        
		// Work loop and scheduling - this is where most of the convolution is done
		// How many partitions to do this vector? (make sure that all partitions are done before we need to do the next fft)
		     
        if (fft_now)
            num_partitions_to_do = (valid_partitions - partitions_done) - 1;
        else
            num_partitions_to_do = (((valid_partitions - 1) * (rw_counter & hop_mask)) / fft_size_halved) - partitions_done;
            
		while (num_partitions_to_do > 0)
		{
			// Calculate buffer wraparounds (if wraparound is in the middle of this set of partitions this loop will run again)
			
			next_partition = (last_partition < num_partitions) ? last_partition : 0;
			last_partition = (next_partition + num_partitions_to_do) > num_partitions ? num_partitions : next_partition + num_partitions_to_do;
			num_partitions_to_do -= last_partition - next_partition;
			
			// Calculate offsets and pointers
			
			DSP_SPLIT_COMPLEX_POINTER_CALC(impulse_temp, impulse_buffer, ((partitions_done + 1) * fft_size_halved));
			DSP_SPLIT_COMPLEX_POINTER_CALC(buffer_temp, input_buffer, (next_partition * fft_size_halved));
			
			// Do processing
			
			for (uintptr_t i = next_partition; i < last_partition; i++)
			{	
				processPartition(buffer_temp, impulse_temp, accum_buffer, fft_size_halved_over_4);
				DSP_SPLIT_COMPLEX_POINTER_CALC(impulse_temp, impulse_temp, fft_size_halved);
				DSP_SPLIT_COMPLEX_POINTER_CALC(buffer_temp, buffer_temp, fft_size_halved);
				partitions_done++;
			}
		}
		
		// FFT processing - this is where we deal with the fft, the first partition and inverse fft
		
		if (fft_now)
		{			
			// Calculate the position to do the fft from/ to and calculate relevant pointers
			
			temp_vpointer = (rw_counter == fft_size) ? fft_buffers[1] : fft_buffers[0];
			DSP_SPLIT_COMPLEX_POINTER_CALC(buffer_temp, input_buffer, (input_position * fft_size_halved));
						
			// Do the fft and put into the input buffer
			
			hisstools_unzip_f((float *) temp_vpointer, &buffer_temp, fft_size_log2);
			hisstools_rfft_f(mFFTSetup, &buffer_temp, fft_size_log2);
			
			// Process first partition here and accumulate the output (we need it now!)
			
			processPartition(buffer_temp, impulse_buffer, accum_buffer, fft_size_halved_over_4);
			
			// Processing done - do inverse fft on the accumulation buffer
			
			hisstools_rifft_f(mFFTSetup, &accum_buffer, fft_size_log2);
			hisstools_zip_f(&accum_buffer, (float *) fft_buffers[2], fft_size_log2);
			
			// Calculate temporary output pointer
			
            temp_vpointer = (rw_counter == fft_size) ? fft_buffers[3] : fft_buffers[3] + fft_size_halved_over_4;
			
			// Scale and store into output buffer (overlap-save) 
			
			for (uintptr_t i = 0; i < fft_size_halved_over_4; i++)
				*(temp_vpointer++) = F32_VEC_MUL_OP(*(fft_buffers[2] + i), vscale_mult);
			
			// Clear accumulation buffer

            memset(accum_buffer.realp, 0, fft_size_halved * sizeof(float));
            memset(accum_buffer.imagp, 0, fft_size_halved * sizeof(float));
            
			// Update rw_counter
			
            rw_counter = rw_counter & (fft_size - 1);
					
			// Set scheduling variables
			            
            valid_partitions = (valid_partitions == num_partitions) ? valid_partitions : valid_partitions + 1;
            input_position = input_position ? input_position - 1 : num_partitions - 1;
            last_partition = input_position + 1;
			partitions_done = 0;
		}
	}
	
	// Write all variables back into the object
	
	mInputPosition = input_position;
	mRWCounter = rw_counter;
    
	mValidPartitions = valid_partitions;
	mPartitionsDone = partitions_done;
	mLastPartition = last_partition;
	
	return true;
}

void HISSTools::DSP::PartitionedConvolve::processPartition(FFT_SPLIT_COMPLEX_F in1, FFT_SPLIT_COMPLEX_F in2, FFT_SPLIT_COMPLEX_F out, uintptr_t numVecs)
{
	vFloat *in_real1 = (vFloat *) in1.realp;
	vFloat *in_imag1 = (vFloat *) in1.imagp;
	vFloat *in_real2 = (vFloat *) in2.realp;
	vFloat *in_imag2 = (vFloat *) in2.imagp;
	vFloat *out_real = (vFloat *) out.realp;
	vFloat *out_imag = (vFloat *) out.imagp;
		
	//	Do Nyquist Calculation and then zero these bins
	
	float nyquist1 = in1.imagp[0];
	float nyquist2 = in2.imagp[0];
	
	out.imagp[0] += nyquist1 * nyquist2;

	in1.imagp[0] = 0.f;
	in2.imagp[0] = 0.f;
	
	// Do other bins (loop unrolled)
	
	for (uintptr_t i = 0; i + 3 < numVecs; i += 4)
	{
		out_real[i+0] = F32_VEC_ADD_OP (out_real[i+0], F32_VEC_SUB_OP (F32_VEC_MUL_OP(in_real1[i+0], in_real2[i+0]), F32_VEC_MUL_OP(in_imag1[i+0], in_imag2[i+0])));
		out_imag[i+0] = F32_VEC_ADD_OP (out_imag[i+0], F32_VEC_ADD_OP (F32_VEC_MUL_OP(in_real1[i+0], in_imag2[i+0]), F32_VEC_MUL_OP(in_imag1[i+0], in_real2[i+0])));
		out_real[i+1] = F32_VEC_ADD_OP (out_real[i+1], F32_VEC_SUB_OP (F32_VEC_MUL_OP(in_real1[i+1], in_real2[i+1]), F32_VEC_MUL_OP(in_imag1[i+1], in_imag2[i+1])));
		out_imag[i+1] = F32_VEC_ADD_OP (out_imag[i+1], F32_VEC_ADD_OP (F32_VEC_MUL_OP(in_real1[i+1], in_imag2[i+1]), F32_VEC_MUL_OP(in_imag1[i+1], in_real2[i+1])));
		out_real[i+2] = F32_VEC_ADD_OP (out_real[i+2], F32_VEC_SUB_OP (F32_VEC_MUL_OP(in_real1[i+2], in_real2[i+2]), F32_VEC_MUL_OP(in_imag1[i+2], in_imag2[i+2])));
		out_imag[i+2] = F32_VEC_ADD_OP (out_imag[i+2], F32_VEC_ADD_OP (F32_VEC_MUL_OP(in_real1[i+2], in_imag2[i+2]), F32_VEC_MUL_OP(in_imag1[i+2], in_real2[i+2])));
		out_real[i+3] = F32_VEC_ADD_OP (out_real[i+3], F32_VEC_SUB_OP (F32_VEC_MUL_OP(in_real1[i+3], in_real2[i+3]), F32_VEC_MUL_OP(in_imag1[i+3], in_imag2[i+3])));
		out_imag[i+3] = F32_VEC_ADD_OP (out_imag[i+3], F32_VEC_ADD_OP (F32_VEC_MUL_OP(in_real1[i+3], in_imag2[i+3]), F32_VEC_MUL_OP(in_imag1[i+3], in_real2[i+3])));
	}

	// Replace nyquist bins
	
    in1.imagp[0] = nyquist1;
	in2.imagp[0] = nyquist2;
}
