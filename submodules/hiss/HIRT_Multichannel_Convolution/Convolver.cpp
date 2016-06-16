

#include "Convolver.h"

#include <stdio.h>
#include <string.h>

HISSTools::DSP::Convolver::Convolver(uint32_t numIns, uint32_t numOuts,
                                     LatencyMode latency)
{
    numIns = numIns < 1 ? 1 : numIns;

    mN2M = true;
    mNumIns = numIns;
    mNumOuts = numOuts;

    for (uint32_t i = 0; i < numOuts; i++)
    {
        mConvolvers.push_back(new HISSTools::DSP::NToMonoConvolve(
            numIns, 16384, (t_convolve_latency_mode)latency));
        mInTemps.push_back(NULL);
    }

    mTemp1 = NULL;
    mTemp2 = NULL;

    alloc_memory_swap(&mTemporaryMemory, (AH_UIntPtr)0, (AH_UIntPtr)0);
}

HISSTools::DSP::Convolver::Convolver(uint32_t numIO, LatencyMode latency)
{
    numIO = numIO < 1 ? 1 : numIO;

    mN2M = false;
    mNumIns = numIO;
    mNumOuts = numIO;

    for (uint32_t i = 0; i < numIO; i++)
    {
        mConvolvers.push_back(
            new NToMonoConvolve(1, 16384, (t_convolve_latency_mode)latency));
        mInTemps.push_back(NULL);
    }

    mTemp1 = NULL;
    mTemp2 = NULL;

    alloc_memory_swap(&mTemporaryMemory, (AH_UIntPtr)0, (AH_UIntPtr)0);
}

HISSTools::DSP::Convolver::~Convolver() throw()
{
    for (uint32_t i = 0; i < mNumOuts; i++) delete mConvolvers[i];

    // FIX - which is correct?

    free_memory_swap(&mTemporaryMemory);
    // ALIGNED_FREE(mInTemps[0]);
}

// Clear IRs

void HISSTools::DSP::Convolver::clear(bool resize)
{
    if (mN2M)
    {
        for (uint32_t i = 0; i < mNumOuts; i++)
            for (uint32_t j = 0; j < mNumIns; j++) clear(j, i, resize);
    }
    else
    {
        for (uint32_t i = 0; i < mNumOuts; i++) clear(i, i, resize);
    }
}

void HISSTools::DSP::Convolver::clear(uint32_t inChan, uint32_t outChan, bool resize)
{
    set(inChan, outChan, (float*)NULL, 0, resize);
}

// DSP Engine Reset

void HISSTools::DSP::Convolver::reset()
{
    if (mN2M)
    {
        for (uint32_t i = 0; i < mNumOuts; i++)
            for (uint32_t j = 0; j < mNumIns; j++) reset(j, i);
    }
    else
    {
        for (uint32_t i = 0; i < mNumOuts; i++) reset(i, i);
    }
}

t_convolve_error HISSTools::DSP::Convolver::reset(uint32_t in_chan, uint32_t out_chan)
{
    // For Parallel operation you must pass the same in/out channel

    if (!mN2M) in_chan -= out_chan;

    if (out_chan < mNumOuts)
        return mConvolvers[out_chan]->reset(in_chan);
    else
        return CONVOLVE_ERR_OUT_CHAN_OUT_OF_RANGE;
}


// Resize and set IR


t_convolve_error HISSTools::DSP::Convolver::resize(uint32_t inChan, uint32_t outChan,
                                                   uintptr_t length)
{
    // For Parallel operation you must pass the same in/out channel

    if (!mN2M) inChan -= outChan;

    if (outChan < mNumOuts)
        return mConvolvers[outChan]->resize(inChan, length);
    else
        return CONVOLVE_ERR_IN_CHAN_OUT_OF_RANGE;
}

t_convolve_error HISSTools::DSP::Convolver::set(uint32_t inChan, uint32_t outChan,
                                                const float* input, uintptr_t length,
                                                bool resize)
{
    // For Parallel operation you must pass the same in/out channel

    if (!mN2M) inChan -= outChan;

    if (outChan < mNumOuts)
        return mConvolvers[outChan]->set(inChan, input, length, resize);
    else
        return CONVOLVE_ERR_OUT_CHAN_OUT_OF_RANGE;
}

t_convolve_error HISSTools::DSP::Convolver::set(uint32_t inChan, uint32_t outChan,
                                                const double* input,
                                                uintptr_t impulseLength, bool resize)
{
    // For Parallel operation you must pass the same in/out channel

    if (!mN2M) inChan -= outChan;

    t_convolve_error err
        = set(inChan, outChan, std::vector<float>(input, input + impulseLength).data(),
              impulseLength, resize);

    return err;
}
// DSP

void HISSTools::DSP::Convolver::process(const double* const* const ins,
                                        double* const* const outs, size_t numIns,
                                        size_t numOuts, size_t numSamples)
{
    setMXCSR();

    void* memPointer = grow_memory_swap(
        &mTemporaryMemory, (mNumIns + 2) * numSamples * sizeof(float), numSamples);
    tempSetup(memPointer, mTemporaryMemory.current_size);

    if (!mTemporaryMemory.current_ptr) numIns = numOuts = 0;

    numIns = numIns > mNumIns ? mNumIns : numIns;
    numOuts = numOuts > mNumOuts ? mNumOuts : numOuts;

    for (uintptr_t i = 0; i < numIns; i++)
        for (AH_UIntPtr j = 0; j < numSamples; j++) mInTemps[i][j] = (float)ins[i][j];

    // FIX - nasty!

    for (AH_UIntPtr i = 0; i < numOuts; i++)
        mConvolvers[i]->process(mN2M ? &mInTemps[0] : (&mInTemps[0]) + i, outs[i], mTemp1,
                                mTemp2, numSamples, numIns);

    unlock_memory_swap(&mTemporaryMemory);

    resetMXCSR();
}

void HISSTools::DSP::Convolver::process(const std::vector<const double*>& ins,
                                        const std::vector<double*>& outs,
                                        size_t numSamples)
{
    process(ins.data(), outs.data(), ins.size(), outs.size(), numSamples);
}

void HISSTools::DSP::Convolver::tempSetup(void* memPointer, AH_UIntPtr maxFrameSize)
{
    mInTemps[0] = (float*)memPointer;

    for (uint32_t i = 1; i < mNumIns; i++) mInTemps[i] = mInTemps[0] + (i * maxFrameSize);

    mTemp1 = mInTemps[mNumIns - 1] + maxFrameSize;
    mTemp2 = mTemp1 + maxFrameSize;
}

void HISSTools::DSP::Convolver::setMXCSR()
{
#if defined(__i386__) || defined(__x86_64__)
    mOldMXCSR = _mm_getcsr();
    _mm_setcsr(mOldMXCSR | 0x8040);
#endif
}

void HISSTools::DSP::Convolver::resetMXCSR()
{
#if defined(__i386__) || defined(__x86_64__)
    _mm_setcsr(mOldMXCSR);
#endif
}
