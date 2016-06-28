
#ifndef _HISSTOOLS_THREADSAFETY_H_
#define _HISSTOOLS_THREADSAFETY_H_

#ifdef __APPLE__
#include <libkern/OSAtomic.h>
typedef int32_t atomic32_t;
#else
#include <windows.h>
typedef volatile long atomic32_t;
#endif

// Crossplatform 32 bit signed atomic operations

namespace HISSTools
{
    struct Atomic32
    {
        Atomic32() : mValue(0) {}
            
        bool atomicSwap(atomic32_t comparand, atomic32_t exchange);
        atomic32_t atomicIncrementBarrier();
        atomic32_t atomicDecrementBarrier();
            
    private:
            
        atomic32_t mValue;
    };
        
    // A generic lock
        
    struct Lock
    {
        virtual ~Lock() {}
            
        virtual void acquire() = 0;
        virtual bool attempt() = 0;
        virtual void release() = 0;
    };
        
    // A helper for acquiring and freeing the lock in a given scope
        
    struct LockScope
    {
        LockScope(Lock* lock);
        ~LockScope();
            
    private:
        
        Lock *mLock;
    };
        
    //  A spin lock
        
    struct SpinLock : public Lock
    {
        void acquire();
        bool attempt();
        void release();
            
    private:
        
        Atomic32 mAtomic;
    };
}

#endif
