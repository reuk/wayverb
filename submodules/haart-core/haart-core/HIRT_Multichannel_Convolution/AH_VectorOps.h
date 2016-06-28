
/*
 *  AH_VectorOps
 *
 *	This header file provides a platform independent interface for SIMD operations (or vector operations).
 *	Code using this interface should compile for SSE or Altivec processors (and is also open to future changes in processor architecture).
 *	The list of operations here is by no means exhasutive, and is updated as needed using a consistent naming scheme.
 *
 *  Copyright 2010 Alex Harker. All rights reserved.
 *
 */

#ifndef _AH_CROSS_PLATFORM_VECTOR_OPS_
#define _AH_CROSS_PLATFORM_VECTOR_OPS_ 

#ifdef __APPLE__
#include <Accelerate/Accelerate.h>
#define FORCE_INLINE				__attribute__ ((always_inline))
#define FORCE_INLINE_DEFINITION
#define ALIGNED_MALLOC malloc
#define ALIGNED_FREE free
#else
#define FORCE_INLINE				__forceinline
#define FORCE_INLINE_DEFINITION		__forceinline;
#include <emmintrin.h>
#include <malloc.h>
#include <sse_mathfun.h>
#define ALIGNED_MALLOC(x)  _aligned_malloc(x, 16)
#define ALIGNED_FREE(x)  _aligned_free(x)
typedef	__m128i	vUInt8;
typedef __m128i vSInt8;
typedef	__m128i vUInt16;
typedef __m128i vSInt16;
typedef __m128i vUInt32;
typedef	__m128i vSInt32;
typedef	__m128i vBool32;
typedef __m128i vUInt64;
typedef __m128i vSInt64;
typedef	__m128  vFloat;
typedef	__m128d vDouble;
#endif

// Test for intel compilation

#ifndef TARGET_INTEL
#if defined( __i386__ ) || defined( __x86_64__ ) || defined(WIN_VERSION)
#define TARGET_INTEL
#endif
#endif

// Runtime test for SSE2

static __inline int SSE2_check()
{
#ifdef __APPLE__
	return 1;
#else
	int SSE2_flag = 0;
	int CPUInfo[4] = {-1, 0, 0, 0};
	int nIds;

	__cpuid(CPUInfo, 0);
	 nIds = CPUInfo[0];

	if (nIds > 0)
	{
		__cpuid(CPUInfo, 1);
		SSE2_flag = (CPUInfo[3] >> 26) & 0x1;
	}
	
	return SSE2_flag;
#endif
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////// Utility macors (non platform-specifc)  ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// The standard compare operations return all bits on, but for use in MSP we probably want values of one
// These routines can be used in this case

static const vFloat Vec_Ops_F32_One = {1.,1.,1.,1.};

#define F32_VEC_EQUAL_MSP_OP(a,b)		F32_VEC_AND_OP(Vec_Ops_F32_One, F32_VEC_EQUAL_OP(a,b)) 
#define F32_VEC_NOTEQUAL_MSP_OP(a,b)	F32_VEC_SUB_OP(Vec_Ops_F32_One, F32_VEC_AND_OP(F32_VEC_EQUAL_OP(a,b), Vec_Ops_F32_One))
#define F32_VEC_GT_MSP_OP(a,b)			F32_VEC_AND_OP(Vec_Ops_F32_One, F32_VEC_GT_OP(a,b))
#define F32_VEC_LT_MSP_OP(a,b)			F32_VEC_AND_OP(Vec_Ops_F32_One, F32_VEC_LT_OP(a,b)) 


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////// Macros for platform-specific vector /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef TARGET_INTEL

static const vDouble Vec_Ops_F64_One = {1.,1.};

#define F64_VEC_EQUAL_MSP_OP(a,b)		F64_VEC_AND_OP(Vec_Ops_F64_One, F64_VEC_EQUAL_OP(a,b)) 
#define F64_VEC_NOTEQUAL_MSP_OP(a,b)	F64_VEC_SUB_OP(Vec_Ops_F64_One, F64_VEC_AND_OP(F64_VEC_EQUAL_OP(a,b), Vec_Ops_F64_One))
#define F64_VEC_GT_MSP_OP(a,b)			F64_VEC_AND_OP(Vec_Ops_F64_One, F64_VEC_GT_OP(a,b))
#define F64_VEC_LT_MSP_OP(a,b)			F64_VEC_AND_OP(Vec_Ops_F64_One, F64_VEC_LT_OP(a,b)) 

/////////////////////////////////////////////////////////////// Intel specific ////////////////////////////////////////////////////////////////

// Floating point 32 bit intrinsics or functions defined here

// Note that the following two set routines are now handled in a platform-specific manner

#define float2vector					_mm_set1_ps
#define long2vector						_mm_set1_epi32

#define F32_VEC_MUL_OP					_mm_mul_ps
#define F32_VEC_DIV_OP					_mm_div_ps
#define F32_VEC_ADD_OP					_mm_add_ps
#define F32_VEC_SUB_OP					_mm_sub_ps

#define F32_VEC_EQUAL_OP(a,b)			_mm_cmpeq_ps(a,b)
#define F32_VEC_GT_OP(a,b)				_mm_cmpgt_ps(a,b)
#define F32_VEC_LT_OP(a,b)				_mm_cmplt_ps(a,b)

#define F32_VEC_MIN_OP					_mm_min_ps
#define F32_VEC_MAX_OP					_mm_max_ps

#define F32_VEC_AND_OP					_mm_and_ps
#define F32_VEC_ANDNOT_OP				_mm_andnot_ps
#define F32_VEC_OR_OP					_mm_or_ps
#define F32_VEC_XOR_OP					_mm_xor_ps
#define F32_VEC_SEL_OP					_mm_sel_ps

#define F32_VEC_SQRT_OP					_mm_sqrt_ps
#define F32_VEC_TRUNC_OP(v)				_mm_cvtepi32_ps(_mm_cvttps_epi32(v))

#define F32_VEC_ULOAD(p)				_mm_loadu_ps((float *) p)
#define F32_VEC_USTORE(p,v)				_mm_storeu_ps((float *) p, v)
#define F32_VEC_MOVE_LO					_mm_movelh_ps
#define F32_VEC_MOVE_HI					_mm_movehl_ps
#define F32_VEC_SHUFFLE					_mm_shuffle_ps

// Conversions from and to 32 bit floating point vectors

#define F32_VEC_INT_TO_FLOAT			_mm_cvtepi32_ps
#define F32_VEC_FLOAT_TO_INT			_mm_cvttps_epi32

// Floating point 32 bit veclib dependencies

#define F32_VEC_TAN_OP					vtanf
#define F32_VEC_COSH_OP					vcoshf
#define F32_VEC_SINH_OP					vsinhf
#define F32_VEC_TANH_OP					vtanhf
#define F32_VEC_ACOS_OP					vacosf
#define F32_VEC_ASIN_OP					vasinf
#define F32_VEC_ATAN_OP					vatanf
#define F32_VEC_ACOSH_OP				vacoshf
#define F32_VEC_ASINH_OP				vasinhf
#define F32_VEC_ATANH_OP				vatanhf

#define F32_VEC_MOD_OP					vfmodf
#define F32_VEC_POW_OP					vpowf

#ifdef __APPLE__
#define F32_VEC_COS_OP					vcosf
#define F32_VEC_SIN_OP					vsinf
#define F32_VEC_LOG_OP					vlogf
#define F32_VEC_EXP_OP					vexpf
#else
#define F32_VEC_COS_OP					cos_ps
#define F32_VEC_SIN_OP					sin_ps
#define F32_VEC_LOG_OP					log_ps
#define F32_VEC_EXP_OP					exp_ps
#endif

// Integer 32 bit intrinsics

#define I32_VEC_ADD_OP					_mm_add_epi32
#define I32_VEC_SUB_OP					_mm_sub_epi32
#define I32_VEC_OR_OP					_mm_or_si128
#define I32_VEC_AND_OP					_mm_and_si128
#define I32_VEC_ADD_OP					_mm_add_epi32
#define I32_VEC_SHUFFLE_OP				_mm_shuffle_epi32

#define I32_VEC_MIN_OP					_mm_min_epi32
#define I32_VEC_MAX_OP					_mm_max_epi32

// Double precision (64 bit) floating point vector ops (intel only - test for intel compile before using)

#define double2vector					_mm_set1_pd

#define F64_VEC_MUL_OP					_mm_mul_pd
#define F64_VEC_DIV_OP					_mm_div_pd
#define F64_VEC_ADD_OP					_mm_add_pd
#define F64_VEC_SUB_OP					_mm_sub_pd

#define F64_VEC_EQUAL_OP(a,b)			_mm_cmpeq_pd(a,b)
#define F64_VEC_GT_OP					_mm_cmpgt_pd
#define F64_VEC_LT_OP					_mm_cmplt_pd

#define F64_VEC_MIN_OP					_mm_min_pd
#define F64_VEC_MAX_OP					_mm_max_pd

#define F64_VEC_AND_OP					_mm_and_pd
#define F64_VEC_OR_OP					_mm_or_pd
#define F64_VEC_XOR_OP					_mm_xor_pd
#define F64_VEC_SEL_OP					_mm_sel_pd

// Conversions from and to 64 bit floating point vectors

#define F64_VEC_FROM_F32				_mm_cvtps_pd
#define F32_VEC_FROM_F64				_mm_cvtpd_ps

#define F64_VEC_INT_TO_FLOAT			_mm_cvtepi32_pd 
#define F64_VEC_FLOAT_TO_INT			_mm_cvtpd_epi32

#define F64_VEC_ULOAD					_mm_loadu_pd
#define F64_VEC_USTORE					_mm_storeu_pd
#define F64_VEC_UNPACK_LO				_mm_unpacklo_pd
#define F64_VEC_UNPACK_HI				_mm_unpackhi_pd
#define F64_VEC_STORE_HI				_mm_storeh_pd
#define F64_VEC_STORE_LO				_mm_storel_pd
#define F64_VEC_SET_BOTH				_mm_set1_pd
#define F64_VEC_SHUFFLE					_mm_shuffle_pd

#define MUL_ZERO_INIT


// Altivec has min / max intrinics for 32 bit signed integers, but on intel this must be done in software (although it is provided under windows)
// These routines are taken directly from the apple SSE migration guide
// The guide can be found at http://developer.apple.com/legacy/mac/library/documentation/Performance/Conceptual/Accelerate_sse_migration/Accelerate_sse_migration.pdf

#ifdef __APPLE__
static __inline vSInt32 _mm_min_epi32(vSInt32 a, vSInt32 b) FORCE_INLINE;
static __inline vSInt32 _mm_min_epi32(vSInt32 a, vSInt32 b) 
{ 
	vSInt32 t = _mm_cmpgt_epi32(a,b);
	return _mm_or_si128( _mm_and_si128(t,b),_mm_andnot_si128(t,a));
}

static __inline vSInt32 _mm_max_epi32(vSInt32 a, vSInt32 b) FORCE_INLINE;
static __inline vSInt32 _mm_max_epi32(vSInt32 a, vSInt32 b) 
{ 
	vSInt32 t = _mm_cmpgt_epi32(a,b);
	return _mm_or_si128( _mm_andnot_si128(t,b),_mm_and_si128(t,a));
}
#endif 

// Altivec has selection intrinics for 32 bit floating point vectors, but on intel this must be done in software
// These routines are taken directly from the apple SSE migration guide
// The guide can be found at http://developer.apple.com/legacy/mac/library/documentation/Performance/Conceptual/Accelerate_sse_migration/Accelerate_sse_migration.pdf

// Inlining is really broken here!!

#ifdef __APPLE__
static __inline vFloat _mm_sel_ps(vFloat a, vFloat b, vFloat mask) FORCE_INLINE;
static __inline vUInt32 _mm_sel_epi32(vUInt32 a, vUInt32 b, vUInt32 mask) FORCE_INLINE;
#endif

static __inline vFloat _mm_sel_ps(vFloat a, vFloat b, vFloat mask) FORCE_INLINE_DEFINITION
{ 
    b = _mm_and_ps(b, mask); 
    a = _mm_andnot_ps(mask, a); 
    return _mm_or_ps(a, b); 
} 

static __inline vUInt32 _mm_sel_epi32(vUInt32 a, vUInt32 b, vUInt32 mask) FORCE_INLINE_DEFINITION
{ 
    b = _mm_and_si128(b, mask); 
    a = _mm_andnot_si128(mask, a); 
    return _mm_or_si128(a, b); 
} 

static __inline vDouble _mm_sel_pd(vDouble a, vDouble b, vDouble mask) FORCE_INLINE;
static __inline vDouble _mm_sel_pd(vDouble a, vDouble b, vDouble mask) 
{ 
    b = _mm_and_pd(b, mask); 
    a = _mm_andnot_pd(mask, a); 
    return _mm_or_pd(a, b); 
} 

#else

////////////////////////////////////////////////////////////// Altivec specific ///////////////////////////////////////////////////////////////

// Floating point 32 bit intrinsics or functions defined here

#define F32_VEC_MUL_OP(v1, v2)			vec_madd(v1,v2, VecMulZero)
#define F32_VEC_DIV_OP					vdivf
#define F32_VEC_ADD_OP					vec_add
#define F32_VEC_SUB_OP					vec_sub

#define F32_VEC_EQUAL_OP(a,b)			vec_cmpeq(a,b) 
#define F32_VEC_GT_OP(a,b)				vec_cmpgt(a,b) 
#define F32_VEC_LT_OP(a,b)				vec_cmplt(a,b)

#define F32_VEC_MIN_OP					vec_min
#define F32_VEC_MAX_OP					vec_max

#define F32_VEC_AND_OP					vec_and
#define F32_VEC_XOR_OP					vecxor
#define F32_VEC_OR_OP					vec_or
#define F32_VEC_SEL_OP					vec_sel

#define F32_VEC_TRUNC_OP				vec_trunc

#define F32_VEC_ULOAD(p)				vec_uload((unsigned char *)p)
#define F32_VEC_USTORE(p, v)			vec_ustore((unsigned char *)p, (vector unsigned char)v)
#define F32_VEC_SHUFFLE					vec_permute

// Floating point 32 bit veclib dependencies

#define F32_VEC_COS_OP					vcosf
#define F32_VEC_SIN_OP					vsinf
#define F32_VEC_TAN_OP					vtanf
#define F32_VEC_COSH_OP					vcoshf
#define F32_VEC_SINH_OP					vsinhf
#define F32_VEC_TANH_OP					vtanhf
#define F32_VEC_ACOS_OP					vacosf
#define F32_VEC_ASIN_OP					vasinf
#define F32_VEC_ATAN_OP					vatanf
#define F32_VEC_ACOSH_OP				vacoshf
#define F32_VEC_ASINH_OP				vasinhf
#define F32_VEC_ATANH_OP				vatanhf

#define F32_VEC_MOD_OP					vfmodf
#define F32_VEC_SQRT_OP					vsqrtf
#define F32_VEC_POW_OP					vpowf
#define F32_VEC_LOG_OP					vlogf
#define F32_VEC_EXP_OP					vexpf

// Integer 32 bit intrinsics

#define I32_VEC_ADD_OP					vec_add
#define I32_VEC_SUB_OP					vec_sub
#define I32_VEC_BIT_AND					vec_and
#define F32_VEC_INT_TO_FLOAT(a)			vec_ctf(a, 0)
#define F32_VEC_FLOAT_TO_INT(a)			vec_cts(a, 0)

#define I32_VEC_MIN_OP					vec_min
#define I32_VEC_MAX_OP					vec_max

#define MUL_ZERO_INIT vFloat			VecMulZero = {0.,0.,0.,0.};

// return a vector filled with a single signed integer value

static __inline vSInt32 long2vector (long longval) FORCE_INLINE;
static __inline vSInt32 long2vector (long longval) FORCE_INLINE_DEFINITION
{
	vSInt32 TheVector = {longval, longval, longval, longval};
	return TheVector;
}

// return a vector filled with a single float value

static __inline vFloat float2vector (float floatval) FORCE_INLINE;
static __inline vFloat float2vector (float floatval) FORCE_INLINE_DEFINITION
{
	vFloat TheVector;
	float *TheFloatArray = (float *) &TheVector;
	
	TheFloatArray[0] = floatval;
	TheFloatArray[1] = floatval;
	TheFloatArray[2] = floatval;
	TheFloatArray[3] = floatval;
	
	return TheVector;
}

// Provide altivec safe misaligned loads and stores (not sure how fast these are)
// These routines are taken directly from the apple SSE migration guide
// The guide can be found at http://developer.apple.com/legacy/mac/library/documentation/Performance/Conceptual/Accelerate_sse_migration/Accelerate_sse_migration.pdf


static inline vFloat vec_uload(unsigned char *target) FORCE_INLINE;
static inline vFloat vec_uload(unsigned char *target)										
{
    vector unsigned char MSQ, LSQ;
    vector unsigned char mask;

    MSQ = vec_ld(0, target);						// most significant quadword
    LSQ = vec_ld(15, target);						// least significant quadword
    mask = vec_lvsl(0, target);						// create the permute mask
    return (vFloat) vec_perm(MSQ, LSQ, mask);		// align the data
}


static __inline void vec_ustore(unsigned char *target, vector unsigned char src) FORCE_INLINE;
static __inline void vec_ustore(unsigned char *target, vector unsigned char src)				
{
    src = vec_perm( src, src, vec_lvsr( 0, target ) );
    vec_ste( (vector unsigned char) src, 0, (unsigned char*) target );
    vec_ste( (vector unsigned short)src,1,(unsigned short*) target );
    vec_ste( (vector unsigned int) src, 3, (unsigned int*) target );
    vec_ste( (vector unsigned int) src, 4, (unsigned int*) target );
    vec_ste( (vector unsigned int) src, 8, (unsigned int*) target );
    vec_ste( (vector unsigned int) src, 12, (unsigned int*) target );
    vec_ste( (vector unsigned short)src,14,(unsigned short*) target );
    vec_ste( (vector unsigned char) src,15,(unsigned char*) target );
}

#endif	/* TARGET_INTEL */

#endif	/* _AH_CROSS_PLATFORM_VECTOR_OPS_ */
