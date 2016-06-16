
#ifndef __FFT_HEADER__
#define __FFT_HEADER__

//#include <cstdint>

// Constants

#define SQRT_2_2 0.70710678118654752440084436210484904

#define FFTLOG2_TRIG_OFFSET ((uintptr_t) 3)
#define PASS_TRIG_OFFSET ((uintptr_t) 2)

// Structures

template <class T> struct Split
{
	T *realp;
	T *imagp;
};

template <class T> struct Setup
{
    unsigned long max_fft_log2;
    Split<T> tables[28];
};

// SIMD Stuff

#ifdef __APPLE__
#define ALIGNED_MALLOC malloc
#define ALIGNED_FREE free
#else
#include <malloc.h>
#define ALIGNED_MALLOC(x)  _aligned_malloc(x, 16)
#define ALIGNED_FREE(x)  _aligned_free(x)
#endif

#ifdef USE_APPLE_FFT
#include <Accelerate/Accelerate.h>
#else
#include <emmintrin.h>
typedef	__m128  vFloat;
typedef	__m128d vDouble;
#endif

// Test for intel compilation

#ifndef TARGET_INTEL
#if defined( __i386__ ) || defined( __x86_64__ ) || defined(WIN_VERSION)
#define TARGET_INTEL
#endif
#endif

// Define for 64 bit float vector in 128bits (2 doubles)

#ifdef TARGET_INTEL
#ifndef VECTOR_F64_128BIT
#define VECTOR_F64_128BIT
#endif
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////// Macros for platform-specific vector /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_INTEL

// Floating point 32 bit intrinsics or functions defined here

#define F32_VEC_MUL_OP					_mm_mul_ps
#define F32_VEC_ADD_OP					_mm_add_ps
#define F32_VEC_SUB_OP					_mm_sub_ps
#define F32_VEC_SHUFFLE					_mm_shuffle_ps

#define F32_SHUFFLE_CONST(z, y, x, w)	((z<<6)|(y<<4)|(x<<2)|w)

// Double precision (64 bit) floating point vector ops

#define F64_VEC_MUL_OP					_mm_mul_pd
#define F64_VEC_ADD_OP					_mm_add_pd
#define F64_VEC_SUB_OP					_mm_sub_pd
#define F64_VEC_SHUFFLE					_mm_shuffle_pd

#define F64_SHUFFLE_CONST(y, x)			((y<<1)|x)

#endif	/* TARGET_INTEL */

#endif
