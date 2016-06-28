
#ifndef _AH_GENERIC_MEMORY_SWAP_
#define _AH_GENERIC_MEMORY_SWAP_ 


#include "AH_Types.h"
#include "AH_VectorOps.h"


#ifdef __APPLE__
#include <libkern/OSAtomic.h>
typedef int32_t t_ah_atomic_32; 
#else
typedef volatile long t_ah_atomic_32;
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#endif


static __inline long Generic_Atomic_Compare_And_Swap_Barrier (t_ah_atomic_32 Comparand, t_ah_atomic_32 Exchange, t_ah_atomic_32 *Destination)
{
#ifdef __APPLE__
	if (OSAtomicCompareAndSwap32Barrier(Comparand, Exchange, (int32_t *) Destination))
#else
	if (InterlockedCompareExchange  (Destination, Exchange, Comparand) == Comparand)
#endif
			return 1;
	return 0;
}

// FIX - this locks around memory assignment - this can be made more efficient by avoiding this at which point spinlocks will be justified....
// Follow the HISSTools C++ design for this.... however use separate freeing locks so that memory is always freed in the assignment thread by waiting...

// All memory assignments are aligned in order that the memory is suitable for vector ops etc.

// Alloc and free routine prototypes

typedef void *(*alloc_method) (AH_UIntPtr size, AH_UIntPtr nom_size);
typedef void (*free_method) (void *);


// Safe memory swap structure

typedef struct _memory_swap
{
	t_ah_atomic_32 lock;
	
	void *current_ptr;
	
	free_method current_free_method;
	
	AH_UIntPtr current_size;
	
} t_memory_swap;


// Free Lock

static __inline void unlock_memory_swap (t_memory_swap *mem_struct)
{
	Generic_Atomic_Compare_And_Swap_Barrier(1, 0, &mem_struct-> lock);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Alloc / Free / Clear


// Alloc - Does not get the lock.... Only ever call once at creation time. It is not safe to use memory until this routine has exited.

static __inline long alloc_memory_swap (t_memory_swap *mem_struct, AH_UIntPtr size, AH_UIntPtr nom_size)
{
	long fail = 0;
	
	mem_struct-> lock = 0;
	
	if (size)
		mem_struct->current_ptr = ALIGNED_MALLOC(size);
	else
		mem_struct->current_ptr = 0;

	if (size && mem_struct->current_ptr)
	{
		mem_struct->current_size = nom_size;
		mem_struct->current_free_method = ALIGNED_FREE;
	}
	else 
	{
		mem_struct->current_size = 0;
		mem_struct->current_free_method = 0;
		
		if (size) 
			fail = 1;
	}
	
	return fail;
}


// Clear - clears memory without getting lock (assumes you already have the lock) 

static __inline void clear_memory_swap (t_memory_swap *mem_struct)
{
	if (mem_struct->current_free_method)
		mem_struct->current_free_method(mem_struct->current_ptr);
}


// Free - frees the memory immediately 

static __inline void free_memory_swap (t_memory_swap *mem_struct)
{
	// Spin on the lock
	
	while (!Generic_Atomic_Compare_And_Swap_Barrier(0, 1, &mem_struct-> lock));
	
	clear_memory_swap(mem_struct);
		
	mem_struct->current_ptr = 0;
	mem_struct->current_size = 0;
	mem_struct->current_free_method = 0;
	
	// This should never fail as this thread has the lock 
	
	Generic_Atomic_Compare_And_Swap_Barrier(1, 0, &mem_struct-> lock);
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Access - these routines are intended to pick up memory, but not allocate it


// Access - this routine will lock to get access to the memory struct and safely return the pointer

static __inline void *access_memory_swap (t_memory_swap *mem_struct, AH_UIntPtr *nom_size)
{
	void *return_ptr;
	
	// Spin on the lock
	
	while (!Generic_Atomic_Compare_And_Swap_Barrier(0, 1, &mem_struct-> lock));
	
	*nom_size = mem_struct->current_size;
	return_ptr = mem_struct->current_ptr;
	
	return return_ptr;	
}


// Attempt - This non-blocking routine will attempt to get the pointer but fail if the pointer is being altered / accessed in another thread

static __inline void *attempt_memory_swap (t_memory_swap *mem_struct, AH_UIntPtr *nom_size)
{
	void *return_ptr = 0;
		
	if (Generic_Atomic_Compare_And_Swap_Barrier(0, 1, &mem_struct->lock))
	{
		*nom_size = mem_struct->current_size;
		return_ptr = mem_struct->current_ptr;
	}
	else
		*nom_size = 0;
	
	return return_ptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Re-allocation routines - these routines allow threadsafe re-allocation of memory


//  Swap Pointer - this routine will lock to get access to the memory struct and place a given ptr and nominal size in the new slots
// This pointer will *NOT* be freed by the memory_swap routines

static __inline void swap_memory_swap (t_memory_swap *mem_struct, void *ptr, long nom_size)
{	
	// Spin on the lock
	
	while (!Generic_Atomic_Compare_And_Swap_Barrier(0, 1, &mem_struct-> lock));
	
	clear_memory_swap(mem_struct);
		
	mem_struct->current_ptr = ptr;
	mem_struct->current_size = nom_size;
	mem_struct->current_free_method = 0;
}


// Grow - this routine will lock to get access to the memory struct and allocate new memory if required, swapping it in safely

static __inline void *grow_memory_swap (t_memory_swap *mem_struct, AH_UIntPtr size, AH_UIntPtr nom_size)
{	
	void *return_ptr;
	
	// Spin on the lock
	
	while (!Generic_Atomic_Compare_And_Swap_Barrier(0, 1, &mem_struct-> lock));

	if (mem_struct->current_size < nom_size)
	{
		clear_memory_swap(mem_struct);
		
		mem_struct->current_ptr = return_ptr = ALIGNED_MALLOC(size);
		mem_struct->current_size = return_ptr ? nom_size: 0;
		mem_struct->current_free_method = ALIGNED_FREE;
	}
	else 
		return_ptr = mem_struct->current_ptr;

	return return_ptr;
}


//  Equal - This routine will lock to get access to the memory struct and allocate new memory unless the sizes are equal, placing the memory in the new slots

static __inline void *equal_memory_swap (t_memory_swap *mem_struct, AH_UIntPtr size, AH_UIntPtr nom_size)
{	
	void *return_ptr; 
	
	// Spin on the lock
	
	while (!Generic_Atomic_Compare_And_Swap_Barrier(0, 1, &mem_struct-> lock));
	
	if (mem_struct->current_size != nom_size)
	{
		clear_memory_swap(mem_struct);
			
		mem_struct->current_ptr = return_ptr = ALIGNED_MALLOC(size);
		mem_struct->current_size = return_ptr ? nom_size: 0;
		mem_struct->current_free_method = ALIGNED_FREE;
	}
	else
		return_ptr = mem_struct->current_ptr;
	
	return return_ptr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Custom allocation and reallocation functions


// Alloc Custom

static __inline long alloc_memory_swap_custom (t_memory_swap *mem_struct, alloc_method alloc_method_ptr, free_method free_method_ptr, AH_UIntPtr size, AH_UIntPtr nom_size)
{
	long fail = 0;
	
	mem_struct-> lock = 0;
	
	if (size)
		mem_struct->current_ptr = alloc_method_ptr(size, nom_size);
	else
		mem_struct->current_ptr = 0;
	
	if (size && mem_struct->current_ptr)
	{
		mem_struct->current_size = nom_size;
		mem_struct->current_free_method = free_method_ptr;
	}
	else 
	{
		mem_struct->current_size = 0;
		mem_struct->current_free_method = 0;
		
		if (size) 
			fail = 1;
	}
	
	return fail;
}


// This routine will lock to get access to the memory struct and allocate new memory if required

static __inline void *grow_memory_swap_custom (t_memory_swap *mem_struct, alloc_method alloc_method_ptr, free_method free_method_ptr, AH_UIntPtr size, AH_UIntPtr nom_size)
{	
	void *return_ptr;
	
	// Spin on the lock
	
	while (!Generic_Atomic_Compare_And_Swap_Barrier(0, 1, &mem_struct-> lock));
	
	if (mem_struct->current_size < nom_size)
	{		
		clear_memory_swap(mem_struct);

		mem_struct->current_ptr = return_ptr = alloc_method_ptr(size, nom_size);
		mem_struct->current_size = return_ptr ? nom_size: 0;
		mem_struct->current_free_method = free_method_ptr;
	}
	else 
		return_ptr = mem_struct->current_ptr;
		
	return return_ptr;
}


// This routine will lock to get access to the memory struct and allocate new memory unless the sizes are equal

static __inline void *equal_memory_swap_custom (t_memory_swap *mem_struct, alloc_method alloc_method_ptr, free_method free_method_ptr, AH_UIntPtr size, AH_UIntPtr nom_size)
{	
	void *return_ptr; 
	
	// Spin on the lock
	
	while (!Generic_Atomic_Compare_And_Swap_Barrier(0, 1, &mem_struct-> lock));
	
	if (mem_struct->current_size != nom_size)
	{
		clear_memory_swap(mem_struct);
		
		mem_struct->current_ptr = return_ptr = alloc_method_ptr(size, nom_size);
		mem_struct->current_size = return_ptr ? nom_size: 0;
		mem_struct->current_free_method = free_method_ptr;
	}
	else 
		return_ptr = mem_struct->current_ptr;
	
	return return_ptr;
}


#endif		/* _AH_GENERIC_MEMORY_SWAP_	*/
