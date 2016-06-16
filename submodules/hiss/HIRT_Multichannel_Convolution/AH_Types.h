
#ifndef _AH_TYPES_
#define _AH_TYPES_

// This needs to be altered to cope with platforms other than windows/mac and compilers other than visual studio and GCC

#ifdef __APPLE__
#if __LP64__
#define AH_64BIT
#endif
#else
#ifdef _WIN64
#define AH_64BIT
#endif 
#endif  


// Pointer-sized Integer definitions

#ifdef AH_64BIT
typedef unsigned long long AH_UIntPtr;
typedef long long AH_SIntPtr;
#else
typedef unsigned long AH_UIntPtr;
typedef long AH_SIntPtr;
#endif

// Integer definitions for given bit sizes

typedef unsigned long long AH_UInt64;
typedef long long AH_SInt64;
typedef unsigned int AH_UInt32;
typedef int AH_SInt32;
typedef unsigned short AH_UInt16;
typedef short AH_SInt16;
typedef unsigned char AH_UInt8;
typedef char AH_SInt8;


// Boolean

typedef int AH_Boolean;

#undef true
#define true 1
#undef false
#define false 0


#endif  /* _AH_TYPES_ */
