
#ifndef FFT_SETUP
#define FFT_SETUP

#include "FFT_Header.h"

// Runtime test for SSE2

bool SSE2Check()
{
#ifdef __APPLE__
    return true;
#else
    int CPUInfo[4] = {-1, 0, 0, 0};
    
    __cpuid(CPUInfo, 0);
    
    if (CPUInfo[0] <= 0)
        return false;
    
    __cpuid(CPUInfo, 1);
    return ((CPUInfo[3] >> 26) & 0x1) ? true : false;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Trig Tables

template <class T> void fillTable(Split<T> *table, uintptr_t length)
{
	T *tableReal = table->realp;
	T *tableImag = table->imagp;
	
	for (uintptr_t i = 0; i < length; i++)
	{
		double angle = -(static_cast<double>(i)) * M_PI / static_cast<double>(length);
		
		*tableReal++ = static_cast<T>(cos(angle));
		*tableImag++ = static_cast<T>(sin(angle));
	}
}

template <class T> Setup<T> *createSetup(uintptr_t max_fft_log2)
{
    // Check for SSE here (this must be called anyway before doing an FFT)
    
    SSE_Exists = SSE2Check();
    
    Setup<T> *setup = (Setup<T> *) ALIGNED_MALLOC(sizeof(Setup<T>));
    
    // Set Max FFT Size
    
    setup->max_fft_log2 = max_fft_log2;
	
    // Create Tables
	
	for (uintptr_t i = FFTLOG2_TRIG_OFFSET; i <= max_fft_log2; i++)
	{
		uintptr_t length = (uintptr_t) 1 << (i - 1);
		
		setup->tables[i - FFTLOG2_TRIG_OFFSET].realp = (T *) ALIGNED_MALLOC(sizeof(T) * 2 * length);
		setup->tables[i - FFTLOG2_TRIG_OFFSET].imagp = setup->tables[i - FFTLOG2_TRIG_OFFSET].realp + length;
		
		fillTable(&setup->tables[i - FFTLOG2_TRIG_OFFSET], length);
	}
	
	return setup;
}	

template <class T> void destroySetup(Setup<T> *setup)
{
    if (!setup)
        return;
    
	for (uintptr_t i = FFTLOG2_TRIG_OFFSET; i <= setup->max_fft_log2; i++)
		ALIGNED_FREE(setup->tables[i - FFTLOG2_TRIG_OFFSET].realp);
	
	ALIGNED_FREE(setup);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Scalar Passes

template <class T> void smallFFT(Split<T> *input, uintptr_t fft_log2)
{
    T r1, r2, r3, r4, r5, r6, r7, r8;
    T i1, i2, i3, i4, i5, i6, i7, i8;
    T t1, t2, t3, t4;
    
    T *r1_ptr = input->realp;
    T *i1_ptr = input->imagp;
    
    if (fft_log2 < 1)
        return;
    
    switch (fft_log2)
    {
        case 1:
            
            r1 = r1_ptr[0];
            r2 = r1_ptr[1];
            i1 = i1_ptr[0];
            i2 = i1_ptr[1];
            
            r1_ptr[0] = r1 + r2;
            r1_ptr[1] = r1 - r2;
            i1_ptr[0] = i1 + i2;
            i1_ptr[1] = i1 - i2;
            
            break;
            
        case 2:
            
            r5 = r1_ptr[0];
            r6 = r1_ptr[1];
            r2 = r1_ptr[2];
            r4 = r1_ptr[3];
            i5 = i1_ptr[0];
            i6 = i1_ptr[1];
            i2 = i1_ptr[2];
            i4 = i1_ptr[3];
            
            // Pass One
            
            r1 = r5 + r2;
            r2 = r5 - r2;
            r3 = r6 + r4;
            r4 = r6 - r4;
            i1 = i5 + i2;
            i2 = i5 - i2;
            i3 = i6 + i4;
            i4 = i6 - i4;
            
            // Pass Two
            
            r1_ptr[0] = r1 + r3;
            r1_ptr[1] = r2 + i4;
            r1_ptr[2] = r1 - r3;
            r1_ptr[3] = r2 - i4;
            i1_ptr[0] = i1 + i3;
            i1_ptr[1] = i2 - r4;
            i1_ptr[2] = i1 - i3;
            i1_ptr[3] = i2 + r4;
            
            break;
            
        case 3:
            
            t1 = r1_ptr[0];
            t3 = r1_ptr[1];
            t2 = r1_ptr[2];
            t4 = r1_ptr[3];
            r2 = r1_ptr[4];
            r6 = r1_ptr[5];
            r4 = r1_ptr[6];
            r8 = r1_ptr[7];
            
            // Pass One
            
            r1 = t1 + r2;
            r2 = t1 - r2;
            r3 = t2 + r4;
            r4 = t2 - r4;
            r5 = t3 + r6;
            r6 = t3 - r6;
            r7 = t4 + r8;
            r8 = t4 - r8;
            
            t1 = i1_ptr[0];
            t3 = i1_ptr[1];
            t2 = i1_ptr[2];
            t4 = i1_ptr[3];
            i2 = i1_ptr[4];
            i6 = i1_ptr[5];
            i4 = i1_ptr[6];
            i8 = i1_ptr[7];
            
            i1 = t1 + i2;
            i2 = t1 - i2;
            i3 = t2 + i4;
            i4 = t2 - i4;
            i5 = t3 + i6;
            i6 = t3 - i6;
            i7 = t4 + i8;
            i8 = t4 - i8;
            
            // Pass Two
            
            r1_ptr[0] = r1 + r3;
            r1_ptr[1] = r2 + i4;
            r1_ptr[2] = r1 - r3;
            r1_ptr[3] = r2 - i4;
            r1_ptr[4] = r5 + r7;
            r1_ptr[5] = r6 + i8;
            r1_ptr[6] = r5 - r7;
            r1_ptr[7] = r6 - i8;
            
            i1_ptr[0] = i1 + i3;
            i1_ptr[1] = i2 - r4;
            i1_ptr[2] = i1 - i3;
            i1_ptr[3] = i2 + r4;
            i1_ptr[4] = i5 + i7;
            i1_ptr[5] = i6 - r8;
            i1_ptr[6] = i5 - i7;
            i1_ptr[7] = i6 + r8;
            
            // Pass Three
            
            pass_3(input, (uintptr_t) 8);
            
            break;
            
    }
}

template <class T> void pass_1_2_reorder(Split<T> *input, uintptr_t length)
{
    uintptr_t offset = length >> 1;
    
    T r1, r2, r3, r4, r5, r6, r7, r8;
    T i1, i2, i3, i4, i5, i6, i7, i8;
    
    T *r1_ptr = input->realp;
    T *i1_ptr = input->imagp;
    T *r2_ptr = r1_ptr + offset;
    T *i2_ptr = i1_ptr + offset;
    
    for (uintptr_t i = 0; i < (length >> 2); i++)
    {
        // Pass One
        
        // Real
        
        r1 = *r1_ptr;
        r2 = *r2_ptr;
        r3 = *(r1_ptr + 1);
        r4 = *(r2_ptr + 1);
        
        *r1_ptr++ = r1 + r2;
        *r1_ptr++ = r1 - r2;
        *r2_ptr++ = r3 + r4;
        *r2_ptr++ = r3 - r4;
        
        // Imaginary
        
        i1 = *i1_ptr;
        i2 = *i2_ptr;
        i3 = *(i1_ptr + 1);
        i4 = *(i2_ptr + 1);
        
        *i1_ptr++ = i1 + i2;
        *i1_ptr++ = i1 - i2;
        *i2_ptr++ = i3 + i4;
        *i2_ptr++ = i3 - i4;
    }
    
    // Pass Two
    
    offset >>= 1;
    
    r1_ptr = input->realp;
    i1_ptr = input->imagp;
    r2_ptr = r1_ptr + offset;
    i2_ptr = i1_ptr + offset;
    
    for (uintptr_t j = 0, loop = (length >> 4); j < 2; j++)
    {
        for (uintptr_t i = 0; i < loop; i++)
        {
            // Get Real
            
            r1 = *(r1_ptr + 0);
            r2 = *(r1_ptr + 1);
            r3 = *(r2_ptr + 0);
            r4 = *(r2_ptr + 1);
            r5 = *(r1_ptr + 2);
            r6 = *(r1_ptr + 3);
            r7 = *(r2_ptr + 2);
            r8 = *(r2_ptr + 3);
            
            // Get Imaginary
            
            i1 = *(i1_ptr + 0);
            i2 = *(i1_ptr + 1);
            i3 = *(i2_ptr + 0);
            i4 = *(i2_ptr + 1);
            i5 = *(i1_ptr + 2);
            i6 = *(i1_ptr + 3);
            i7 = *(i2_ptr + 2);
            i8 = *(i2_ptr + 3);
            
            // Store Real
            
            *r1_ptr++ = r1 + r3;
            *r1_ptr++ = r2 + i4;
            *r1_ptr++ = r1 - r3;
            *r1_ptr++ = r2 - i4;
            *r2_ptr++ = r5 + r7;
            *r2_ptr++ = r6 + i8;
            *r2_ptr++ = r5 - r7;
            *r2_ptr++ = r6 - i8;
            
            // Store Imaginary
            
            *i1_ptr++ = i1 + i3;
            *i1_ptr++ = i2 - r4;
            *i1_ptr++ = i1 - i3;
            *i1_ptr++ = i2 + r4;
            *i2_ptr++ = i5 + i7;
            *i2_ptr++ = i6 - r8;
            *i2_ptr++ = i5 - i7;
            *i2_ptr++ = i6 + r8;
        }
        
        r1_ptr += offset;
        i1_ptr += offset;
        r2_ptr += offset;
        i2_ptr += offset;
    }
}

template <class T> void pass_3(Split<T> *input, uintptr_t length)
{
    T sqrt_2_2 = static_cast<T>(SQRT_2_2);
    T r1, r2, r3, r4, r5, r6, r7, r8;
    T i1, i2, i3, i4, i5, i6, i7, i8;
    T t1, t2;
    
    T *r1_ptr = input->realp;
    T *i1_ptr = input->imagp;
    
    for (uintptr_t i = 0; i < length >> 3; i++)
    {
        // Pass Three
        
        r1 = *(r1_ptr + 0);
        r2 = *(r1_ptr + 1);
        r3 = *(r1_ptr + 2);
        r4 = *(r1_ptr + 3);
        r5 = *(r1_ptr + 4);
        r6 = *(r1_ptr + 5);
        r7 = *(r1_ptr + 6);
        r8 = *(r1_ptr + 7);
        
        i1 = *(i1_ptr + 0);
        i2 = *(i1_ptr + 1);
        i3 = *(i1_ptr + 2);
        i4 = *(i1_ptr + 3);
        i5 = *(i1_ptr + 4);
        i6 = *(i1_ptr + 5);
        i7 = *(i1_ptr + 6);
        i8 = *(i1_ptr + 7);
        
        // Real
        
        t1 = sqrt_2_2 * (r6 + i6);
        t2 = sqrt_2_2 * (i8 - r8);
        
        *r1_ptr++ = r1 + r5;
        *r1_ptr++ = r2 + t1;
        *r1_ptr++ = r3 + i7;
        *r1_ptr++ = r4 + t2;
        *r1_ptr++ = r1 - r5;
        *r1_ptr++ = r2 - t1;
        *r1_ptr++ = r3 - i7;
        *r1_ptr++ = r4 - t2;
        
        // Imaginary
        
        t1 = sqrt_2_2 * (i6 - r6);
        t2 = -sqrt_2_2 * (r8 + i8);
        
        *i1_ptr++ = i1 + i5;
        *i1_ptr++ = i2 + t1;
        *i1_ptr++ = i3 - r7;
        *i1_ptr++ = i4 + t2;
        *i1_ptr++ = i1 - i5;
        *i1_ptr++ = i2 - t1;
        *i1_ptr++ = i3 + r7;
        *i1_ptr++ = i4 - t2;
    }
}

template <class T> void pass_3_reorder(Split<T> *input, uintptr_t length, uintptr_t fft_log2)
{
    uintptr_t offset = (uintptr_t) 1 << (fft_log2 - 3);
    
    T sqrt_2_2 = static_cast<T>(SQRT_2_2);
    T r1, r2, r3, r4, r5, r6, r7, r8, r9, r10, r11, r12, r13, r14, r15, r16;
    T i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16;
    T t1, t2, t3, t4;
    
    T *r1_ptr = input->realp;
    T *i1_ptr = input->imagp;
    T *r2_ptr = r1_ptr + offset;
    T *i2_ptr = i1_ptr + offset;
    
    for (uintptr_t j = 0, loop = length >> 6; j < 4; j++)
    {
        for (uintptr_t i = 0; i < loop; i++)
        {
            // Get Real
            
            r1 = *(r1_ptr + 0);
            r2 = *(r1_ptr + 1);
            r3 = *(r1_ptr + 2);
            r4 = *(r1_ptr + 3);
            
            r9 = *(r1_ptr + 4);
            r10 = *(r1_ptr + 5);
            r11 = *(r1_ptr + 6);
            r12 = *(r1_ptr + 7);
            
            r5 = *(r2_ptr + 0);
            r6 = *(r2_ptr + 1);
            r7 = *(r2_ptr + 2);
            r8 = *(r2_ptr + 3);
            
            r13 = *(r2_ptr + 4);
            r14 = *(r2_ptr + 5);
            r15 = *(r2_ptr + 6);
            r16 = *(r2_ptr + 7);
            
            // Get Imaginary
            
            i1 = *(i1_ptr + 0);
            i2 = *(i1_ptr + 1);
            i3 = *(i1_ptr + 2);
            i4 = *(i1_ptr + 3);
            
            i9 = *(i1_ptr + 4);
            i10 = *(i1_ptr + 5);
            i11 = *(i1_ptr + 6);
            i12 = *(i1_ptr + 7);
            
            i5 = *(i2_ptr + 0);
            i6 = *(i2_ptr + 1);
            i7 = *(i2_ptr + 2);
            i8 = *(i2_ptr + 3);
            
            i13 = *(i2_ptr + 4);
            i14 = *(i2_ptr + 5);
            i15 = *(i2_ptr + 6);
            i16 = *(i2_ptr + 7);
            
            t1 = sqrt_2_2 * (r6 + i6);
            t2 = sqrt_2_2 * (i8 - r8);
            t3 = sqrt_2_2 * (r14 + i14);
            t4 = sqrt_2_2 * (i16 - r16);
            
            *r1_ptr++ = r1 + r5;
            *r1_ptr++ = r2 + t1;
            *r1_ptr++ = r3 + i7;
            *r1_ptr++ = r4 + t2;
            *r1_ptr++ = r1 - r5;
            *r1_ptr++ = r2 - t1;
            *r1_ptr++ = r3 - i7;
            *r1_ptr++ = r4 - t2;
            
            *r2_ptr++ = r9 + r13;
            *r2_ptr++ = r10 + t3;
            *r2_ptr++ = r11 + i15;
            *r2_ptr++ = r12 + t4;
            *r2_ptr++ = r9 - r13;
            *r2_ptr++ = r10 - t3;
            *r2_ptr++ = r11 - i15;
            *r2_ptr++ = r12 - t4;
            
            // Imaginary
            
            t1 = sqrt_2_2 * (i6 - r6);
            t2 = -sqrt_2_2 * (r8 + i8);
            t3 = sqrt_2_2 * (i14 - r14);
            t4 = -sqrt_2_2 * (r16 + i16);
            
            *i1_ptr++ = i1 + i5;
            *i1_ptr++ = i2 + t1;
            *i1_ptr++ = i3 - r7;
            *i1_ptr++ = i4 + t2;
            *i1_ptr++ = i1 - i5;
            *i1_ptr++ = i2 - t1;
            *i1_ptr++ = i3 + r7;
            *i1_ptr++ = i4 - t2;
            
            *i2_ptr++ = i9 + i13;
            *i2_ptr++ = i10 + t3;
            *i2_ptr++ = i11 - r15;
            *i2_ptr++ = i12 + t4;
            *i2_ptr++ = i9 - i13;
            *i2_ptr++ = i10 - t3;
            *i2_ptr++ = i11 + r15;
            *i2_ptr++ = i12 - t4;
        }
        
        r1_ptr += offset;
        i1_ptr += offset;
        r2_ptr += offset;
        i2_ptr += offset;
    }
}

template <class T> void pass_trig_table_reorder(Split<T> *input, Setup<T> *setup, uintptr_t length, uintptr_t pass)
{
    uintptr_t size = 2 << pass;
    uintptr_t incr = size >> 1;
    uintptr_t loop = size;
    uintptr_t offset = length >> (pass + 1);
    uintptr_t outerLoop = ((length >> 1) / size) / ((uintptr_t) 1 << pass);
    
    T r1, r2, r3, r4, r5;
    T i1, i2, i3, i4, i5;
    T twidReal, twidImag;
    
    T *r1_ptr = input->realp;
    T *i1_ptr = input->imagp;
    T *r2_ptr = r1_ptr + incr;
    T *i2_ptr = i1_ptr + incr;
    T *r3_ptr = r1_ptr + offset;
    T *i3_ptr = i1_ptr + offset;
    T *r4_ptr = r3_ptr + incr;
    T *i4_ptr = i3_ptr + incr;
    
    for (uintptr_t j = 0, i = 0; i < length >> 1; loop += size)
    {
        T *tableReal = setup->tables[pass - PASS_TRIG_OFFSET].realp;
        T *tableImag = setup->tables[pass - PASS_TRIG_OFFSET].imagp;
        
        for (; i < loop; i += 2)
        {
            // Get input
            
            r1 = *r1_ptr;
            i1 = *i1_ptr;
            r2 = *r3_ptr;
            i2 = *i3_ptr;
            
            // Multiply by twiddle
            
            twidReal = *tableReal++;
            twidImag = *tableImag++;
            
            r5 = (r2 * twidReal) - (i2 * twidImag);
            i5 = (r2 * twidImag) + (i2 * twidReal);
            
            // Get input
            
            r3 = *r2_ptr;
            i3 = *i2_ptr;
            r4 = *r4_ptr;
            i4 = *i4_ptr;
            
            // Store output (same pos as inputs)
            
            *r1_ptr++ = r1 + r5;
            *i1_ptr++ = i1 + i5;
            
            *r2_ptr++ = r1 - r5;
            *i2_ptr++ = i1 - i5;
            
            // Multiply by twiddle
            
            r5 = (r4 * twidReal) - (i4 * twidImag);
            i5 = (r4 * twidImag) + (i4 * twidReal);
            
            // Store output (same pos as inputs)
            
            *r3_ptr++ = r3 + r5;
            *i3_ptr++ = i3 + i5;
            
            *r4_ptr++ = r3 - r5;
            *i4_ptr++ = i3 - i5;
        }
        
        r1_ptr += incr;
        r2_ptr += incr;
        r3_ptr += incr;
        r4_ptr += incr;
        i1_ptr += incr;
        i2_ptr += incr;
        i3_ptr += incr;
        i4_ptr += incr;
        
        if (!(++j % outerLoop))
        {
            r1_ptr += offset;
            r2_ptr += offset;
            r3_ptr += offset;
            r4_ptr += offset;
            i1_ptr += offset;
            i2_ptr += offset;
            i3_ptr += offset;
            i4_ptr += offset;
            
        }
    }
}

template <class T> void pass_trig_table(Split<T> *input, Setup<T> *setup, uintptr_t length, uintptr_t pass)
{
    uintptr_t size = 2 << pass;
    uintptr_t incr = size >> 1;
    uintptr_t loop = size;
    
    T r0, r1, r2;
    T i0, i1, i2;
    T twidReal, twidImag;
    
    T *r1_ptr = input->realp;
    T *i1_ptr = input->imagp;
    T *r2_ptr = r1_ptr + (size >> 1);
    T *i2_ptr = i1_ptr + (size >> 1);
    
    for (uintptr_t i = 0; i < length; loop += size)
    {
        T *tr_ptr = setup->tables[pass - PASS_TRIG_OFFSET].realp;
        T *ti_ptr = setup->tables[pass - PASS_TRIG_OFFSET].imagp;
        
        for (; i < loop; i += 2)
        {
            twidReal = *tr_ptr++;
            twidImag = *ti_ptr++;
            
            // Get input
            
            r1 = *r1_ptr;
            i1 = *i1_ptr;
            r2 = *r2_ptr;
            i2 = *i2_ptr;
            
            // Multiply by twiddle
            
            r0 = (r2 * twidReal) - (i2 * twidImag);
            i0 = (r2 * twidImag) + (i2 * twidReal);
            
            // Store output (same pos as inputs)
            
            *r1_ptr++ = r1 + r0;
            *i1_ptr++ = i1 + i0;
            
            *r2_ptr++ = r1 - r0;
            *i2_ptr++ = i1 - i0;
        }
        
        
        r1_ptr += incr;
        r2_ptr += incr;
        i1_ptr += incr;
        i2_ptr += incr;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Real FFTs

template <class T> void unzipComplex(T *input, Split<T> *output, uintptr_t half_length)
{
    T *realp = output->realp;
    T *imagp = output->imagp;
    
    for (uintptr_t i = 0; i < half_length; i++)
    {
        *realp++ = *input++;
        *imagp++ = *input++;
    }
}


template <class T> void zipComplex(Split<T> *input, T *output, uintptr_t half_length)
{
    T *realp = input->realp;
    T *imagp = input->imagp;
    
    for (uintptr_t i = 0; i < half_length; i++)
    {
        *output++ = *realp++;
        *output++ = *imagp++;
    }
}

template <class T> void smallRealFFT(Split<T> *input, uintptr_t fft_log2, bool ifft)
{
    T scale = static_cast<T>(2.0);
    T r1, r2, r3, r4, i1, i2;
    
    T *r1_ptr = input->realp;
    T *i1_ptr = input->imagp;
    
    if (fft_log2 < 1)
        return;
    
    if (ifft)
        scale = static_cast<T>(1.0);
    
    switch (fft_log2)
    {
        case 1:
            
            r1 = r1_ptr[0];
            r2 = i1_ptr[0];
            
            r1_ptr[0] = (r1 + r2) * scale;
            i1_ptr[0] = (r1 - r2) * scale;
            
            break;
            
        case 2:
            
            if (!ifft)
            {
                r3 = r1_ptr[0];
                r4 = r1_ptr[1];
                i1 = i1_ptr[0];
                i2 = i1_ptr[1];
                
                // Pass One
                
                r1 = r3 + r4;
                r2 = r3 - r4;
                r3 = i1 + i2;
                r4 = i1 - i2;
                
                // Pass Two
                
                r1_ptr[0] = (r1 + r3) * 2;
                r1_ptr[1] = r2 * 2;
                i1_ptr[0] = (r1 - r3) * 2;
                i1_ptr[1] = -r4 * 2;
            }
            else
            {
                i1 = r1_ptr[0];
                r2 = r1_ptr[1] + r1_ptr[1];
                i2 = i1_ptr[0];
                r4 = -i1_ptr[1] - i1_ptr[1];
                
                // Pass One
                
                r1 = i1 + i2;
                r3 = i1 - i2;
                
                // Pass Two
                
                r1_ptr[0] = r1 + r2;
                r1_ptr[1] = r1 - r2;
                i1_ptr[0] = r3 + r4;
                i1_ptr[1] = r3 - r4;
            }
            
            break;
    }
}

template <class T> void pass_real_trig_table(Split<T> *input, Setup<T> *setup, uintptr_t fft_log2, bool ifft)
{
    uintptr_t length = (uintptr_t) 1 << (fft_log2 - 1);
    uintptr_t lengthM1 = length - 1;
    
    T flip = static_cast<T>(1.0);
    T r1, r2, r3, r4;
    T i1, i2, i3, i4;
    T t1, t2;
    T twidReal1, twidImag1;
    
    T *r1_ptr = input->realp;
    T *i1_ptr = input->imagp;
    T *r2_ptr = r1_ptr + lengthM1;
    T *i2_ptr = i1_ptr + lengthM1;
    T *tableReal = setup->tables[fft_log2 - FFTLOG2_TRIG_OFFSET].realp;
    T *tableImag = setup->tables[fft_log2 - FFTLOG2_TRIG_OFFSET].imagp;
    
    if (ifft)
        flip = static_cast<T>(-1.0);
    
    // Do DC and Nyquist (note the the complex values can be considered periodic)
    
    tableReal++;
    tableImag++;
    
    r1 = *r1_ptr;
    i1 = *i1_ptr;
    
    t1 = r1 + i1;
    t2 = r1 - i1;
    
    if (!ifft)
    {
        t1 *= static_cast<T>(2.0);
        t2 *= static_cast<T>(2.0);
    }
    
    *r1_ptr++ = t1;
    *i1_ptr++ = t2;
    
    // N.B. - The last time through this loop will write the same values twice to the same places
    // N.B. - In this case: t1 == 0, i4 == 0, r1_ptr == r2_ptr, i1_ptr == i2_ptr
    
    for (uintptr_t i = 0; i < (length >> 1); i++)
    {
        twidReal1 = flip * *tableReal++;
        twidImag1 = *tableImag++;
        
        // Get input
        
        r1 = *r1_ptr;
        i1 = *i1_ptr;
        r2 = *r2_ptr;
        i2 = *i2_ptr;
        
        r3 = r1 + r2;
        i3 = i1 + i2;
        r4 = r1 - r2;
        i4 = i1 - i2;
        
        t1 = (twidReal1 * i3) + (twidImag1 * r4);
        t2 = (twidReal1 * -r4) + (twidImag1 * i3);
        
        // Store output (same pos as inputs)
        
        *r1_ptr++ = r3 + t1;
        *i1_ptr++ = t2 + i4;
        
        // Store output (same pos as inputs)
        
        *r2_ptr-- = r3 - t1;
        *i2_ptr-- = t2 - i4;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// SIMD Passes

#ifdef VECTOR_F64_128BIT

void pass_1_2_reorder_simd(Split<double> *input, uintptr_t length)
{
    vDouble r1, r2, r3, r4, r5, r6, r7, r8;
    vDouble i1, i2, i3, i4, i5, i6, i7, i8;
    vDouble t1, t2, t3, t4;
    
    vDouble *r1_ptr = reinterpret_cast<vDouble *>(input->realp);
    vDouble *i1_ptr = reinterpret_cast<vDouble *>(input->imagp);
    vDouble *r2_ptr = r1_ptr + (length >> 3);
    vDouble *i2_ptr = i1_ptr + (length >> 3);
    vDouble *r3_ptr = r2_ptr + (length >> 3);
    vDouble *i3_ptr = i2_ptr + (length >> 3);
    vDouble *r4_ptr = r3_ptr + (length >> 3);
    vDouble *i4_ptr = i3_ptr + (length >> 3);
    
    for (uintptr_t i = 0; i < length >> 4; i++)
    {
        t1 = *r1_ptr;
        t2 = *(r1_ptr + 1);
        t3 = *r2_ptr;
        t4 = *(r2_ptr + 1);
        r5 = *r3_ptr;
        r6 = *(r3_ptr + 1);
        r7 = *r4_ptr;
        r8 = *(r4_ptr + 1);
        
        r1 = F64_VEC_ADD_OP(t1, r5);
        r2 = F64_VEC_ADD_OP(t2, r6);
        r3 = F64_VEC_ADD_OP(t3, r7);
        r4 = F64_VEC_ADD_OP(t4, r8);
        r5 = F64_VEC_SUB_OP(t1, r5);
        r6 = F64_VEC_SUB_OP(t2, r6);
        r7 = F64_VEC_SUB_OP(t3, r7);
        r8 = F64_VEC_SUB_OP(t4, r8);
        
        t1 = *i1_ptr;
        t2 = *(i1_ptr + 1);
        t3 = *i2_ptr;
        t4 = *(i2_ptr + 1);
        i5 = *i3_ptr;
        i6 = *(i3_ptr + 1);
        i7 = *i4_ptr;
        i8 = *(i4_ptr + 1);
        
        i1 = F64_VEC_ADD_OP(t1, i5);
        i2 = F64_VEC_ADD_OP(t2, i6);
        i3 = F64_VEC_ADD_OP(t3, i7);
        i4 = F64_VEC_ADD_OP(t4, i8);
        i5 = F64_VEC_SUB_OP(t1, i5);
        i6 = F64_VEC_SUB_OP(t2, i6);
        i7 = F64_VEC_SUB_OP(t3, i7);
        i8 = F64_VEC_SUB_OP(t4, i8);
        
        t1 = F64_VEC_ADD_OP(r1, r3);
        t2 = F64_VEC_ADD_OP(r2, r4);
        t3 = F64_VEC_SUB_OP(r1, r3);
        t4 = F64_VEC_SUB_OP(r2, r4);
        r1 = F64_VEC_ADD_OP(r5, i7);
        r2 = F64_VEC_ADD_OP(r6, i8);
        r3 = F64_VEC_SUB_OP(r5, i7);
        r4 = F64_VEC_SUB_OP(r6, i8);
        
        *r1_ptr++ = F64_VEC_SHUFFLE(t1, r1, F64_SHUFFLE_CONST(0, 0));
        *r1_ptr++ = F64_VEC_SHUFFLE(t3, r3, F64_SHUFFLE_CONST(0, 0));
        *r2_ptr++ = F64_VEC_SHUFFLE(t2, r2, F64_SHUFFLE_CONST(0, 0));
        *r2_ptr++ = F64_VEC_SHUFFLE(t4, r4, F64_SHUFFLE_CONST(0, 0));
        *r3_ptr++ = F64_VEC_SHUFFLE(t1, r1, F64_SHUFFLE_CONST(1, 1));
        *r3_ptr++ = F64_VEC_SHUFFLE(t3, r3, F64_SHUFFLE_CONST(1, 1));
        *r4_ptr++ = F64_VEC_SHUFFLE(t2, r2, F64_SHUFFLE_CONST(1, 1));
        *r4_ptr++ = F64_VEC_SHUFFLE(t4, r4, F64_SHUFFLE_CONST(1, 1));
        
        t1 = F64_VEC_ADD_OP(i1, i3);
        t2 = F64_VEC_ADD_OP(i2, i4);
        t3 = F64_VEC_SUB_OP(i1, i3);
        t4 = F64_VEC_SUB_OP(i2, i4);
        i1 = F64_VEC_SUB_OP(i5, r7);
        i2 = F64_VEC_SUB_OP(i6, r8);
        i3 = F64_VEC_ADD_OP(i5, r7);
        i4 = F64_VEC_ADD_OP(i6, r8);
        
        *i1_ptr++ = F64_VEC_SHUFFLE(t1, i1, F64_SHUFFLE_CONST(0, 0));
        *i1_ptr++ = F64_VEC_SHUFFLE(t3, i3, F64_SHUFFLE_CONST(0, 0));
        *i2_ptr++ = F64_VEC_SHUFFLE(t2, i2, F64_SHUFFLE_CONST(0, 0));
        *i2_ptr++ = F64_VEC_SHUFFLE(t4, i4, F64_SHUFFLE_CONST(0, 0));
        *i3_ptr++ = F64_VEC_SHUFFLE(t1, i1, F64_SHUFFLE_CONST(1, 1));
        *i3_ptr++ = F64_VEC_SHUFFLE(t3, i3, F64_SHUFFLE_CONST(1, 1));
        *i4_ptr++ = F64_VEC_SHUFFLE(t2, i2, F64_SHUFFLE_CONST(1, 1));
        *i4_ptr++ = F64_VEC_SHUFFLE(t4, i4, F64_SHUFFLE_CONST(1, 1));
    }
}

void pass_1_2_reorder_simd(Split<float> *input, uintptr_t length)
{
    vFloat r1, r2, r3, r4, r5, r6, r7, r8;
    vFloat i1, i2, i3, i4, i5, i6, i7, i8;
    
    vFloat *r1_ptr = reinterpret_cast<vFloat *>(input->realp);
    vFloat *r2_ptr = r1_ptr + (length >> 4);
    vFloat *r3_ptr = r2_ptr + (length >> 4);
    vFloat *r4_ptr = r3_ptr + (length >> 4);
    vFloat *i1_ptr = reinterpret_cast<vFloat *>(input->imagp);
    vFloat *i2_ptr = i1_ptr + (length >> 4);
    vFloat *i3_ptr = i2_ptr + (length >> 4);
    vFloat *i4_ptr = i3_ptr + (length >> 4);
    
    for (uintptr_t i = 0; i < length >> 4; i++)
    {
        r5 = *r1_ptr;
        r6 = *r2_ptr;
        r3 = *r3_ptr;
        r4 = *r4_ptr;
        
        i5 = *i1_ptr;
        i6 = *i2_ptr;
        i3 = *i3_ptr;
        i4 = *i4_ptr;
        
        r1 = F32_VEC_ADD_OP(r5, r3);
        r2 = F32_VEC_ADD_OP(r6, r4);
        r3 = F32_VEC_SUB_OP(r5, r3);
        r4 = F32_VEC_SUB_OP(r6, r4);
        
        i1 = F32_VEC_ADD_OP(i5, i3);
        i2 = F32_VEC_ADD_OP(i6, i4);
        i3 = F32_VEC_SUB_OP(i5, i3);
        i4 = F32_VEC_SUB_OP(i6, i4);
        
        r5 = F32_VEC_ADD_OP(r1, r2);
        r6 = F32_VEC_SUB_OP(r1, r2);
        r7 = F32_VEC_ADD_OP(r3, i4);
        r8 = F32_VEC_SUB_OP(r3, i4);
        
        i5 = F32_VEC_ADD_OP(i1, i2);
        i6 = F32_VEC_SUB_OP(i1, i2);
        i7 = F32_VEC_SUB_OP(i3, r4);
        i8 = F32_VEC_ADD_OP(i3, r4);
        
        r1 = F32_VEC_SHUFFLE(r5, r7, F32_SHUFFLE_CONST(1, 0, 1, 0));
        r2 = F32_VEC_SHUFFLE(r5, r7, F32_SHUFFLE_CONST(3, 2, 3, 2));
        r3 = F32_VEC_SHUFFLE(r6, r8, F32_SHUFFLE_CONST(1, 0, 1, 0));
        r4 = F32_VEC_SHUFFLE(r6, r8, F32_SHUFFLE_CONST(3, 2, 3, 2));
        
        *r1_ptr++ = F32_VEC_SHUFFLE(r1, r3, F32_SHUFFLE_CONST(2, 0, 2, 0));
        *r2_ptr++ = F32_VEC_SHUFFLE(r2, r4, F32_SHUFFLE_CONST(2, 0, 2, 0));
        *r3_ptr++ = F32_VEC_SHUFFLE(r1, r3, F32_SHUFFLE_CONST(3, 1, 3, 1));
        *r4_ptr++ = F32_VEC_SHUFFLE(r2, r4, F32_SHUFFLE_CONST(3, 1, 3, 1));
        
        i1 = F32_VEC_SHUFFLE(i5, i7, F32_SHUFFLE_CONST(1, 0, 1, 0));
        i2 = F32_VEC_SHUFFLE(i5, i7, F32_SHUFFLE_CONST(3, 2, 3, 2));
        i3 = F32_VEC_SHUFFLE(i6, i8, F32_SHUFFLE_CONST(1, 0, 1, 0));
        i4 = F32_VEC_SHUFFLE(i6, i8, F32_SHUFFLE_CONST(3, 2, 3, 2));
        
        *i1_ptr++ = F32_VEC_SHUFFLE(i1, i3, F32_SHUFFLE_CONST(2, 0, 2, 0));
        *i2_ptr++ = F32_VEC_SHUFFLE(i2, i4, F32_SHUFFLE_CONST(2, 0, 2, 0));
        *i3_ptr++ = F32_VEC_SHUFFLE(i1, i3, F32_SHUFFLE_CONST(3, 1, 3, 1));
        *i4_ptr++ = F32_VEC_SHUFFLE(i2, i4, F32_SHUFFLE_CONST(3, 1, 3, 1));
    }
}



void pass_3_reorder_simd(Split<double> *input, uintptr_t length)
{
    uintptr_t offset = length >> 4;
    uintptr_t outerLoop = length >> 6;
    
    vDouble r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    vDouble i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    vDouble twidReal1 = { 1.0,  SQRT_2_2};
    vDouble twidImag1 = { 0.0, -SQRT_2_2};
    vDouble twidReal2 = { 0.0, -SQRT_2_2};
    vDouble twidImag2 = {-1.0, -SQRT_2_2};
    
    vDouble *r1_ptr = reinterpret_cast<vDouble *>(input->realp);
    vDouble *i1_ptr = reinterpret_cast<vDouble *>(input->imagp);
    vDouble *r2_ptr = r1_ptr + offset;
    vDouble *i2_ptr = i1_ptr + offset;
    
    for (uintptr_t i = 0, j = 0; i < length >> 1; i += 8)
    {
        // Get input
        
        r1 = *r1_ptr;
        r5 = *(r1_ptr + 1);
        r2 = *r2_ptr;
        r6 = *(r2_ptr + 1);
        i1 = *i1_ptr;
        i5 = *(i1_ptr + 1);
        i2 = *i2_ptr;
        i6 = *(i2_ptr + 1);
        
        // Multiply by twiddle
        
        r9 = F64_VEC_SUB_OP(F64_VEC_MUL_OP(r2, twidReal1), F64_VEC_MUL_OP(i2, twidImag1));
        i9 = F64_VEC_ADD_OP(F64_VEC_MUL_OP(r2, twidImag1), F64_VEC_MUL_OP(i2, twidReal1));
        r10 = F64_VEC_SUB_OP(F64_VEC_MUL_OP(r6, twidReal2), F64_VEC_MUL_OP(i6, twidImag2));
        i10 = F64_VEC_ADD_OP(F64_VEC_MUL_OP(r6, twidImag2), F64_VEC_MUL_OP(i6, twidReal2));
        
        // Get input
        
        r3 = *(r1_ptr + 2);
        r7 = *(r1_ptr + 3);
        r4 = *(r2_ptr + 2);
        r8 = *(r2_ptr + 3);
        i3 = *(i1_ptr + 2);
        i7 = *(i1_ptr + 3);
        i4 = *(i2_ptr + 2);
        i8 = *(i2_ptr + 3);
        
        // Store output (swapping as necessary)
        
        *r1_ptr = F64_VEC_ADD_OP(r1, r9);
        *(r1_ptr + 1) = F64_VEC_ADD_OP(r5, r10);
        *i1_ptr = F64_VEC_ADD_OP(i1, i9);
        *(i1_ptr + 1) = F64_VEC_ADD_OP(i5, i10);
        
        *(r1_ptr++ + 2) = F64_VEC_SUB_OP(r1, r9);
        *(r1_ptr++ + 2) = F64_VEC_SUB_OP(r5, r10);
        *(i1_ptr++ + 2) = F64_VEC_SUB_OP(i1, i9);
        *(i1_ptr++ + 2) = F64_VEC_SUB_OP(i5, i10);
        
        // Multiply by twiddle
        
        r9 = F64_VEC_SUB_OP(F64_VEC_MUL_OP(r4, twidReal1), F64_VEC_MUL_OP(i4, twidImag1));
        i9 = F64_VEC_ADD_OP(F64_VEC_MUL_OP(r4, twidImag1), F64_VEC_MUL_OP(i4, twidReal1));
        r10 = F64_VEC_SUB_OP(F64_VEC_MUL_OP(r8, twidReal2), F64_VEC_MUL_OP(i8, twidImag2));
        i10 = F64_VEC_ADD_OP(F64_VEC_MUL_OP(r8, twidImag2), F64_VEC_MUL_OP(i8, twidReal2));
        
        // Store output (swapping as necessary)
        
        *r2_ptr = F64_VEC_ADD_OP(r3, r9);
        *(r2_ptr + 1) = F64_VEC_ADD_OP(r7, r10);
        *i2_ptr = F64_VEC_ADD_OP(i3, i9);
        *(i2_ptr + 1) = F64_VEC_ADD_OP(i7, i10);
        
        *(r2_ptr++ + 2) = F64_VEC_SUB_OP(r3, r9);
        *(r2_ptr++ + 2) = F64_VEC_SUB_OP(r7, r10);
        *(i2_ptr++ + 2) = F64_VEC_SUB_OP(i3, i9);
        *(i2_ptr++ + 2) = F64_VEC_SUB_OP(i7, i10);
        
        r1_ptr += 2;
        r2_ptr += 2;
        i1_ptr += 2;
        i2_ptr += 2;
        
        if (!(++j % outerLoop))
        {
            r1_ptr += offset;
            r2_ptr += offset;
            i1_ptr += offset;
            i2_ptr += offset;
        }
    }
}

void pass_3_reorder_simd(Split<float> *input, uintptr_t length)
{
    uintptr_t offset = length >> 5;
    uintptr_t outerLoop = length >> 6;
    
    vFloat r1, r2, r3, r4, r5;
    vFloat i1, i2, i3, i4, i5;
    
    vFloat twidReal = {1.f, static_cast<float>( SQRT_2_2),  0.f, static_cast<float>(-SQRT_2_2)};
    vFloat twidImag = {0.f, static_cast<float>(-SQRT_2_2), -1.f, static_cast<float>(-SQRT_2_2)};
    
    vFloat *r1_ptr = reinterpret_cast<vFloat *>(input->realp);
    vFloat *i1_ptr = reinterpret_cast<vFloat *>(input->imagp);
    vFloat *r2_ptr = r1_ptr + offset;
    vFloat *i2_ptr = i1_ptr + offset;
    
    for (uintptr_t i = 0, j = 0; i < length >> 1; i += 8)
    {
        // Get input
        
        r1 = *r1_ptr;
        i1 = *i1_ptr;
        r3 = *(r1_ptr + 1);
        i3 = *(i1_ptr + 1);
        r2 = *r2_ptr;
        i2 = *i2_ptr;
        r4 = *(r2_ptr + 1);
        i4 = *(i2_ptr + 1);
        
        // Multiply by twiddle
        
        r5 = F32_VEC_SUB_OP(F32_VEC_MUL_OP(r2, twidReal), F32_VEC_MUL_OP(i2, twidImag));
        i5 = F32_VEC_ADD_OP(F32_VEC_MUL_OP(r2, twidImag), F32_VEC_MUL_OP(i2, twidReal));
        
        // Store output (swapping as necessary)
        
        *r1_ptr = F32_VEC_ADD_OP(r1, r5);
        *i1_ptr = F32_VEC_ADD_OP(i1, i5);
        
        *(r1_ptr + 1) = F32_VEC_SUB_OP(r1, r5);
        *(i1_ptr + 1) = F32_VEC_SUB_OP(i1, i5);
        
        // Multiply by twiddle
        
        r5 = F32_VEC_SUB_OP(F32_VEC_MUL_OP(r4, twidReal), F32_VEC_MUL_OP(i4, twidImag));
        i5 = F32_VEC_ADD_OP(F32_VEC_MUL_OP(r4, twidImag), F32_VEC_MUL_OP(i4, twidReal));
        
        // Store output (swapping as necessary)
        
        *r2_ptr = F32_VEC_ADD_OP(r3, r5);
        *i2_ptr = F32_VEC_ADD_OP(i3, i5);
        
        *(r2_ptr + 1) = F32_VEC_SUB_OP(r3, r5);
        *(i2_ptr + 1) = F32_VEC_SUB_OP(i3, i5);
        
        r1_ptr += 2;
        r2_ptr += 2;
        i1_ptr += 2;
        i2_ptr += 2;
        
        if (!(++j % outerLoop))
        {
            r1_ptr += offset;
            r2_ptr += offset;
            i1_ptr += offset;
            i2_ptr += offset;
        }
    }
}

void pass_trig_table_reorder_simd(Split<double> *input, Setup<double> *setup, uintptr_t length, uintptr_t pass)
{
    uintptr_t size = 2 << pass;
    uintptr_t incr = size >> 2;
    uintptr_t loop = size;
    uintptr_t offset = length >> (pass + 2);
    uintptr_t outerLoop = ((length >> 1) / size) / ((uintptr_t) 1 << pass);
    
    vDouble r1, r2, r3, r4, r5;
    vDouble i1, i2, i3, i4, i5;
    vDouble twidReal, twidImag;
    
    vDouble *r1_ptr = reinterpret_cast<vDouble *>(input->realp);
    vDouble *i1_ptr = reinterpret_cast<vDouble *>(input->imagp);
    vDouble *r2_ptr = r1_ptr + offset;
    vDouble *i2_ptr = i1_ptr + offset;
    
    for (uintptr_t i = 0, j = 0; i < (length >> 1); loop += size)
    {
        vDouble *tableReal = reinterpret_cast<vDouble *>(setup->tables[pass - PASS_TRIG_OFFSET].realp);
        vDouble *tableImag = reinterpret_cast<vDouble *>(setup->tables[pass - PASS_TRIG_OFFSET].imagp);
        
        for (; i < loop; i += 4)
        {
            // Get input
            
            r1 = *r1_ptr;
            i1 = *i1_ptr;
            r2 = *r2_ptr;
            i2 = *i2_ptr;
            
            // Multiply by twiddle
            
            twidReal = *tableReal++;
            twidImag = *tableImag++;
            
            r5 = F64_VEC_SUB_OP(F64_VEC_MUL_OP(r2, twidReal), F64_VEC_MUL_OP(i2, twidImag));
            i5 = F64_VEC_ADD_OP(F64_VEC_MUL_OP(r2, twidImag), F64_VEC_MUL_OP(i2, twidReal));
            
            // Get input
            
            r3 = *(r1_ptr + incr);
            i3 = *(i1_ptr + incr);
            r4 = *(r2_ptr + incr);
            i4 = *(i2_ptr + incr);
            
            // Store output (swapping as necessary)
            
            *r1_ptr = F64_VEC_ADD_OP(r1, r5);
            *i1_ptr = F64_VEC_ADD_OP(i1, i5);
            
            *(r1_ptr++ + incr) = F64_VEC_SUB_OP(r1, r5);
            *(i1_ptr++ + incr) = F64_VEC_SUB_OP(i1, i5);
            
            // Multiply by twiddle
            
            r5 = F64_VEC_SUB_OP(F64_VEC_MUL_OP(r4, twidReal), F64_VEC_MUL_OP(i4, twidImag));
            i5 = F64_VEC_ADD_OP(F64_VEC_MUL_OP(r4, twidImag), F64_VEC_MUL_OP(i4, twidReal));
            
            // Store output (swapping as necessary)
            
            *r2_ptr = F64_VEC_ADD_OP(r3, r5);
            *i2_ptr = F64_VEC_ADD_OP(i3, i5);
            
            *(r2_ptr++ + incr) = F64_VEC_SUB_OP(r3, r5);
            *(i2_ptr++ + incr) = F64_VEC_SUB_OP(i3, i5);
        }
        
        r1_ptr += incr;
        r2_ptr += incr;
        i1_ptr += incr;
        i2_ptr += incr;
        
        if (!(++j % outerLoop))
        {
            r1_ptr += offset;
            r2_ptr += offset;
            i1_ptr += offset;
            i2_ptr += offset;
        }
    }
}

void pass_trig_table_reorder_simd(Split<float> *input, Setup<float> *setup, uintptr_t length, uintptr_t pass)
{
    uintptr_t size = 2 << pass;
    uintptr_t incr = size >> 3;
    uintptr_t loop = size;
    uintptr_t offset = length >> (pass + 3);
    uintptr_t outerLoop = ((length >> 1) / size) / ((uintptr_t) 1 << pass);
    
    vFloat r1, r2, r3, r4, r5;
    vFloat i1, i2, i3, i4, i5;
    vFloat twidReal, twidImag;
    
    vFloat *r1_ptr = reinterpret_cast<vFloat *>(input->realp);
    vFloat *i1_ptr = reinterpret_cast<vFloat *>(input->imagp);
    vFloat *r2_ptr = r1_ptr + offset;
    vFloat *i2_ptr = i1_ptr + offset;
    
    for (uintptr_t i = 0, j = 0; i < length >> 1; loop += size)
    {
        vFloat *tableReal = reinterpret_cast<vFloat *>(setup->tables[pass - PASS_TRIG_OFFSET].realp);
        vFloat *tableImag = reinterpret_cast<vFloat *>(setup->tables[pass - PASS_TRIG_OFFSET].imagp);
        
        for (; i < loop; i += 8)
        {
            // Get input
            
            r1 = *r1_ptr;
            i1 = *i1_ptr;
            r2 = *r2_ptr;
            i2 = *i2_ptr;
            
            // Get Twiddle
            
            twidReal = *tableReal++;
            twidImag = *tableImag++;
            
            // Multiply by twiddle
            
            r5 = F32_VEC_SUB_OP(F32_VEC_MUL_OP(r2, twidReal), F32_VEC_MUL_OP(i2, twidImag));
            i5 = F32_VEC_ADD_OP(F32_VEC_MUL_OP(r2, twidImag), F32_VEC_MUL_OP(i2, twidReal));
            
            // Get input
            
            r3 = *(r1_ptr + incr);
            i3 = *(i1_ptr + incr);
            r4 = *(r2_ptr + incr);
            i4 = *(i2_ptr + incr);
            
            // Store output (swapping as necessary)
            
            *r1_ptr = F32_VEC_ADD_OP(r1, r5);
            *i1_ptr = F32_VEC_ADD_OP(i1, i5);
            
            *(r1_ptr++ + incr) = F32_VEC_SUB_OP(r1, r5);
            *(i1_ptr++ + incr) = F32_VEC_SUB_OP(i1, i5);
            
            // Multiply by twiddle
            
            r5 = F32_VEC_SUB_OP(F32_VEC_MUL_OP(r4, twidReal), F32_VEC_MUL_OP(i4, twidImag));
            i5 = F32_VEC_ADD_OP(F32_VEC_MUL_OP(r4, twidImag), F32_VEC_MUL_OP(i4, twidReal));
            
            // Store output (swapping as necessary)
            
            *r2_ptr = F32_VEC_ADD_OP(r3, r5);
            *i2_ptr = F32_VEC_ADD_OP(i3, i5);
            
            *(r2_ptr++ + incr) = F32_VEC_SUB_OP(r3, r5);
            *(i2_ptr++ + incr) = F32_VEC_SUB_OP(i3, i5);
        }
        
        r1_ptr += incr;
        r2_ptr += incr;
        i1_ptr += incr;
        i2_ptr += incr;
        
        if (!(++j % outerLoop))
        {
            r1_ptr += offset;
            r2_ptr += offset;
            i1_ptr += offset;
            i2_ptr += offset;
        }
    }
}

void pass_trig_table_simd(Split<double> *input, Setup<double> *setup, uintptr_t length, uintptr_t pass)
{
    uintptr_t size = 2 << pass;
    uintptr_t incr = size >> 2;
    uintptr_t loop = size;
    
    vDouble r1, r2, r3;
    vDouble i1, i2, i3;
    vDouble twidReal, twidImag;
    
    vDouble *r1_ptr = reinterpret_cast<vDouble *>(input->realp);
    vDouble *i1_ptr = reinterpret_cast<vDouble *>(input->imagp);
    vDouble *r2_ptr = r1_ptr + incr;
    vDouble *i2_ptr = i1_ptr + incr;
    
    for (uintptr_t i = 0; i < length; loop += size)
    {
        vDouble *tableReal = reinterpret_cast<vDouble *>(setup->tables[pass - PASS_TRIG_OFFSET].realp);
        vDouble *tableImag = reinterpret_cast<vDouble *>(setup->tables[pass - PASS_TRIG_OFFSET].imagp);
        
        for (; i < loop; i += 4)
        {
            // Get input
            
            r1 = *r1_ptr;
            i1 = *i1_ptr;
            r2 = *r2_ptr;
            i2 = *i2_ptr;
            
            // Multiply by twiddle
            
            twidReal = *tableReal++;
            twidImag = *tableImag++;
            
            r3 = F64_VEC_SUB_OP(F64_VEC_MUL_OP(r2, twidReal), F64_VEC_MUL_OP(i2, twidImag));
            i3 = F64_VEC_ADD_OP(F64_VEC_MUL_OP(r2, twidImag), F64_VEC_MUL_OP(i2, twidReal));
            
            // Store output (same pos as inputs)
            
            *r1_ptr++ = F64_VEC_ADD_OP(r1, r3);
            *i1_ptr++ = F64_VEC_ADD_OP(i1, i3);
            
            *r2_ptr++ = F64_VEC_SUB_OP(r1, r3);
            *i2_ptr++ = F64_VEC_SUB_OP(i1, i3);
        }
        
        r1_ptr += incr;
        r2_ptr += incr;
        i1_ptr += incr;
        i2_ptr += incr;
    }
}

void pass_trig_table_simd(Split<float> *input, Setup<float> *setup, uintptr_t length, uintptr_t pass)
{
    uintptr_t size = 2 << pass;
    uintptr_t incr = size >> 3;
    uintptr_t loop = size;
    
    vFloat r1, r2, r3;
    vFloat i1, i2, i3;
    vFloat twidReal, twidImag;
    
    vFloat *r1_ptr = reinterpret_cast<vFloat *>(input->realp);
    vFloat *i1_ptr = reinterpret_cast<vFloat *>(input->imagp);
    vFloat *r2_ptr = r1_ptr + incr;
    vFloat *i2_ptr = i1_ptr + incr;
    
    for (uintptr_t i = 0; i < length; loop += size)
    {
        vFloat *tableReal = reinterpret_cast<vFloat *>(setup->tables[pass - PASS_TRIG_OFFSET].realp);
        vFloat *tableImag = reinterpret_cast<vFloat *>(setup->tables[pass - PASS_TRIG_OFFSET].imagp);
        
        for (; i < loop; i += 8)
        {
            // Get input
            
            r1 = *r1_ptr;
            i1 = *i1_ptr;
            r2 = *r2_ptr;
            i2 = *i2_ptr;
            
            // Get Twiddle
            
            twidReal = *tableReal++;
            twidImag = *tableImag++;
            
            // Multiply by twiddle
            
            r3 = F32_VEC_SUB_OP(F32_VEC_MUL_OP(r2, twidReal), F32_VEC_MUL_OP(i2, twidImag));
            i3 = F32_VEC_ADD_OP(F32_VEC_MUL_OP(r2, twidImag), F32_VEC_MUL_OP(i2, twidReal));
            
            // Store output (same pos as inputs)
            
            *r1_ptr++ = F32_VEC_ADD_OP(r1, r3);
            *i1_ptr++ = F32_VEC_ADD_OP(i1, i3);
            
            *r2_ptr++ = F32_VEC_SUB_OP(r1, r3);
            *i2_ptr++ = F32_VEC_SUB_OP(i1, i3);
        }
        
        r1_ptr += incr;
        r2_ptr += incr;
        i1_ptr += incr;
        i2_ptr += incr;
    }
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Main Calls

template <class T> void hisstools_fft(Split<T> *input, Setup<T> *setup, uintptr_t fft_log2)
{
    uintptr_t length = (uintptr_t) 1 << fft_log2;
    uintptr_t i;
    
    if (fft_log2 < 4)
    {
        smallFFT(input, fft_log2);
        return;
    }
    
#ifdef VECTOR_F64_128BIT
    if ((uintptr_t) input->realp % 16 || (uintptr_t) input->imagp % 16 || !SSE_Exists)
#endif
    {
        pass_1_2_reorder(input, length);
        
        if (fft_log2 > 5)
            pass_3_reorder(input, length, fft_log2);
        else
            pass_3(input, length);
        
        for (i = 3; i < (fft_log2 >> 1); i++)
            pass_trig_table_reorder(input, setup, length, i);
        
        for (; i < fft_log2; i++)
            pass_trig_table(input, setup, length, i);
    }
#ifdef VECTOR_F64_128BIT
    else
    {
        pass_1_2_reorder_simd(input, length);
        
        if (fft_log2 > 5)
            pass_3_reorder_simd(input, length);
        else
            pass_3(input, length);
        
        for (i = 3; i < (fft_log2 >> 1); i++)
            pass_trig_table_reorder_simd(input, setup, length, i);
        
        for (; i < fft_log2; i++)
            pass_trig_table_simd(input, setup, length, i);
    }
#endif
}

template <class T>void hisstools_ifft(Split<T> *input, Setup<T> *setup, uintptr_t fft_log2)
{
    Split<T> swap;
    
    swap.realp = input->imagp;
    swap.imagp = input->realp;
    
    hisstools_fft(&swap, setup, fft_log2);
}

template <class T>void hisstools_rfft(Split<T> *input, Setup<T> *setup, uintptr_t fft_log2)
{
    if (fft_log2 < 3)
    {
        smallRealFFT(input, fft_log2, false);
        return;
    }
    
    hisstools_fft(input, setup, fft_log2 - 1);
    pass_real_trig_table(input, setup, fft_log2, false);
}


template <class T>void hisstools_rifft(Split<T> *input, Setup<T> *setup, uintptr_t fft_log2)
{
    if (fft_log2 < 3)
    {
        smallRealFFT(input, fft_log2, true);
        return;
    }
    
    pass_real_trig_table(input, setup, fft_log2, true);
    hisstools_ifft(input, setup, fft_log2 - 1);
}

#endif

