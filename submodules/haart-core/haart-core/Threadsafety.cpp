
#include "Threadsafety.h"

// Crossplatform 32 bit signed atomic operations

bool HISSTools::Atomic32::atomicSwap(atomic32_t comparand, atomic32_t exchange)
{
#ifdef __APPLE__
    return OSAtomicCompareAndSwap32Barrier(comparand, exchange, &mValue);
#else
    return (InterlockedCompareExchange (&mValue, exchange, comparand) == comparand);
#endif
}

atomic32_t HISSTools::Atomic32::atomicIncrementBarrier()
{
#ifdef __APPLE__
    return OSAtomicIncrement32Barrier(&mValue);
#else
    return InterlockedIncrement(&mValue);
#endif
}

atomic32_t HISSTools::Atomic32::atomicDecrementBarrier()
{
#ifdef __APPLE__
    return OSAtomicDecrement32Barrier(&mValue);
#else
    return InterlockedDecrement(&mValue);
#endif
}

// A helper for acquiring and freeing the lock in a given scope
        
HISSTools::LockScope::LockScope(Lock* lock)
{
    mLock = lock;
    if (mLock)
        mLock->acquire();
}

HISSTools::LockScope::~LockScope()
{
    if (mLock)
        mLock->release();
    mLock = NULL;
}

//  A spin lock

void HISSTools::SpinLock::acquire()
{
    while(mAtomic.atomicSwap(0, 1) == false);
}

bool HISSTools::SpinLock::attempt()
{
    return mAtomic.atomicSwap(0, 1);
}

void HISSTools::SpinLock::release()
{
    mAtomic.atomicSwap(1, 0);
}
