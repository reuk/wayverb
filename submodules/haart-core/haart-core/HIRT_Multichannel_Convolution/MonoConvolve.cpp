

#include "MonoConvolve.h"


// Allocation Helpers

void *largest_partition_time_alloc(AH_UIntPtr size, AH_UIntPtr )
{
    return new HISSTools::PartitionedConvolve(16384, (size > 16384 ? size : 16384) - 8192, 8192, 0);
}

void *largest_partition_fft1_alloc(AH_UIntPtr size, AH_UIntPtr )
{
	return new HISSTools::PartitionedConvolve(16384, (size > 16384 ? size : 16384) - 8064, 8064, 0);
}

void *largest_partition_fft2_alloc(AH_UIntPtr size, AH_UIntPtr )
{
	return new HISSTools::PartitionedConvolve(16384, (size > 16384 ? size : 16384) - 7680, 7680, 0);
}

void largest_partition_free(void *largePartition)
{
	delete (HISSTools::PartitionedConvolve *) largePartition;
}


// Constructure and Deconstructor


HISSTools::MonoConvolve::MonoConvolve(uintptr_t maxLength, t_convolve_latency_mode latency)
{
	long fail = 0;
	
	latency = latency > (t_convolve_latency_mode) 2 ? (t_convolve_latency_mode) 2 : latency;

	switch (latency)
	{
		case CONVOLVE_LATENCY_ZERO:

			mTime1 = new TimeDomainConvolve(0, 128);
			mPart1 = new PartitionedConvolve(256, 384, 128, 384);
			mPart2 = new PartitionedConvolve(1024, 1536, 512, 1536);
			mPart3 = new PartitionedConvolve(4096, 6144, 2048, 6144);
			fail = alloc_memory_swap_custom(&mPart4, largest_partition_time_alloc, largest_partition_free, maxLength, maxLength);
			break;
		
		case CONVOLVE_LATENCY_SHORT:
			
			mTime1 = NULL;
			mPart1 = new PartitionedConvolve(256, 384, 0, 384);
			mPart2 = new PartitionedConvolve(1024, 1536, 384, 1536);
			mPart3 = new PartitionedConvolve(4096, 6144, 1920, 6144);
			fail = alloc_memory_swap_custom(&mPart4, largest_partition_fft1_alloc, largest_partition_free, maxLength, maxLength);
			break;
			
		case CONVOLVE_LATENCY_MEDIUM:
			
			mTime1 = NULL;
			mPart1 = NULL;
			mPart2 = new PartitionedConvolve(1024, 1536, 0, 1536);
			mPart3 = new PartitionedConvolve(4096, 6144, 1536, 6144);
			fail = alloc_memory_swap_custom(&mPart4, largest_partition_fft2_alloc, largest_partition_free, maxLength, maxLength);
			break;
	}

	mLatency = latency;
	mLength = 0;
}

HISSTools::MonoConvolve::~MonoConvolve()
{
    delete mTime1;
    delete mPart1;
    delete mPart2;
    delete mPart3;
    free_memory_swap(&mPart4);
}

HISSTools::PartitionedConvolve *HISSTools::MonoConvolve::resize(uintptr_t length, bool keep_lock)
{
	PartitionedConvolve *return_part = 0;
    alloc_method allocator = largest_partition_time_alloc;
    
    mLength = 0;
	
	switch (mLatency)
	{
        case CONVOLVE_LATENCY_ZERO:         allocator = largest_partition_time_alloc;       break;
        case CONVOLVE_LATENCY_SHORT:        allocator = largest_partition_fft1_alloc;       break;
        case CONVOLVE_LATENCY_MEDIUM:       allocator = largest_partition_fft2_alloc;       break;
	}	
	
    return_part = (PartitionedConvolve *) equal_memory_swap_custom(&mPart4, allocator, largest_partition_free, length, length);
    
	if (keep_lock == false)
		unlock_memory_swap(&mPart4);
	
	return return_part;
}


t_convolve_error HISSTools::MonoConvolve::set(const float *input, uintptr_t length, bool resizeFlag)
{	
	PartitionedConvolve *part4 = NULL;

    // FIX - what is the purpose of this?
	
    AH_UIntPtr max_impulse;
	
	mLength = 0;

	// Lock first to ensure that audio finishes processing before we replace
	
	if (resizeFlag)
	{
		part4 = resize(length, true);
		max_impulse = length;
	}
	else 
		part4 = (PartitionedConvolve *)access_memory_swap(&mPart4, &max_impulse);

	if (part4)
	{
		if (mLatency < 1)
			mTime1->set(input, length);
		if (mLatency < 2)
			mPart1->set(input, length);
		mPart2->set(input, length);
		mPart3->set(input, length);
		part4->set(input, length);
	}

	mLength = length;

	unlock_memory_swap(&mPart4);
	
	if (length && !part4)
		return CONVOLVE_ERR_MEM_UNAVAILABLE;
	
	if (length > max_impulse)
		return CONVOLVE_ERR_MEM_ALLOC_TOO_SMALL;
		
	return CONVOLVE_ERR_NONE;
}

t_convolve_error HISSTools::MonoConvolve::reset()
{
    AH_UIntPtr max_impulse;
    
    // Lock first to ensure that audio finishes processing before we replace
    
    PartitionedConvolve *part4 = (PartitionedConvolve *) access_memory_swap(&mPart4, &max_impulse);
    
    if (part4)
    {
        if (mLatency < 1)
            mTime1->reset();
        if (mLatency < 2)
            mPart1->reset();
        mPart2->reset();
        mPart3->reset();
        part4->reset();
    }
    
    unlock_memory_swap(&mPart4);
    
    return CONVOLVE_ERR_NONE;
}


void HISSTools::MonoConvolve::sum(float *out, float *add, uintptr_t numSamples)
{
    if ((numSamples % 4) || (((uintptr_t) out) % 16) || (((uintptr_t) add) % 16))
    {
        for (uintptr_t i = 0; i < numSamples; i++)
            *out++ += *add++;
    }
    else
    {
        vFloat *vout = (vFloat *) out;
        vFloat *vadd = (vFloat *) add;
        
        for (uintptr_t i = 0; i < (numSamples >> 2); i++, vout++)
            *vout = F32_VEC_ADD_OP(*vout, *vadd++);
    }
}

void HISSTools::MonoConvolve::process(const float *in, float *temp, float *out, uintptr_t numSamples)
{		
	AH_UIntPtr maxLength = 0;
	PartitionedConvolve *part4 = (PartitionedConvolve *) attempt_memory_swap(&mPart4, &maxLength);
	
	// N.B. This function DOES NOT zero the output buffer as this is done elsewhere
	
	if (mLength && mLength <= maxLength && part4)
	{
		if (mTime1)
		{
			mTime1->process(in, temp, numSamples);
            sum(out, temp, numSamples);
		}
		if (mPart1)
		{
            if (mPart1->process(in, temp, numSamples) == true)
				sum(out, temp, numSamples);
		}
		if (mPart2->process(in, temp, numSamples) == true)
			sum(out, temp, numSamples);
		if (mPart3->process(in, temp, numSamples) == true)
			sum(out, temp, numSamples);
		if (part4->process(in, temp, numSamples) == true)
			sum(out, temp, numSamples);
	}
	
	if (part4)
		unlock_memory_swap(&mPart4);
}
