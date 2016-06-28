
#include <algorithm>

#include "IRMeasure.h"


// FIX - memory assignment exceptions really matter here...

// Main Class

HISSTools::IRMeasure::IRMeasure() {}

HISSTools::IRMeasure::~IRMeasure()
{
    cancel();
}

 // Start/Stop

HISSTools::IRMeasure::ExcitationAndBuffer::ExcitationAndBuffer(Excitation * excitation,
                                                                    double * recordBuffer)
        : mExcitation(excitation), mRecordBuffer(recordBuffer) {
}

void HISSTools::IRMeasure::start(const Excitation &excitationSignal)
{
    ExcitationAndBuffer eab(excitationSignal.clone(), new double[excitationSignal.getNumIns() * excitationSignal.getMeasurementLength()]);
    eab.mExcitation->reset();
    swap(eab);
}

void HISSTools::IRMeasure::test(const Excitation &excitationSignal)
{
    ExcitationAndBuffer eab(excitationSignal.clone(), NULL);
    eab.mExcitation->reset();
    swap(eab);
}

void HISSTools::IRMeasure::cancel()
{
    ExcitationAndBuffer eab;
    swap(eab);
}

// Audio Engine Reset

void HISSTools::IRMeasure::reset(double /*samplingRate*/)
{
    cancel();
}

// Excitation Position

uintptr_t HISSTools::IRMeasure::getPosition()
{
    LockScope lock(&mLock);
 
    return getPositionLocked();
}

double HISSTools::IRMeasure::getNormPosition()
{
    LockScope lock(&mLock);
 
    return getNormPositionLocked();
}

bool HISSTools::IRMeasure::isDone()
{
    LockScope lock(&mLock);
 
    return isDoneLocked();
}

 // DSP

void HISSTools::IRMeasure::process(const double *const *const ins, double *const *const outs, uintptr_t /*numIns*/, uintptr_t numOuts, uintptr_t numSamples)
{
    LockScope lock(&mLock);
    
    const std::auto_ptr<Excitation>& excitation = mExcitationAndBuffer.mExcitation;
    if (excitation.get() == NULL)
    {
        for (uintptr_t i = 0; i < numOuts; i++)
            memset(outs[i], 0, numSamples * sizeof(double));

        return;
    }

    // Record Input
    
    const std::auto_ptr<double>& recordBuffer = mExcitationAndBuffer.mRecordBuffer;
    if (recordBuffer.get() != NULL && !isDoneLocked())
    {
        for (uintptr_t i = 0; i < excitation->getNumIns(); i++)
        {
            double *chanBuffer  = recordBuffer.get() + (i * excitation->getMeasurementLength());

            for (uintptr_t j = 0, t = excitation->getPosition(); j < numSamples && t < excitation->getMeasurementLength(); j++, t++)
                chanBuffer[t] = ins[i][j];
        }
    }

    // Generate Output

    excitation->generateBlock(outs, numOuts, numSamples);
}

void HISSTools::IRMeasure::process(const std::vector<double *>& ins, const std::vector<double *>& outs, uintptr_t numSamples)
{
    process(ins.data(), outs.data(), ins.size(), outs.size(), numSamples);
}

// Process and Retrieve Output

HISSTools::IRRetrieve *HISSTools::IRMeasure::processRecording()
{
    ExcitationAndBuffer eab;
    
    if (!swap(eab, true))
        return NULL;
    
    return new IRRetrieve(eab.mExcitation.release(), eab.mRecordBuffer.release(), mDeconvolutionSpecification);
}

// Swap Memory Safely

bool HISSTools::IRMeasure::swap(ExcitationAndBuffer& rhs, bool requireDone)
{
    LockScope lock(&mLock);

    if (requireDone && !isDoneLocked())
        return false;
        
    std::swap(rhs, mExcitationAndBuffer);
    
    return true;
}

// Internal Getters (the lock should be acquired before calling)

uintptr_t HISSTools::IRMeasure::getPositionLocked()
{
    const std::auto_ptr<Excitation> & excitation = mExcitationAndBuffer.mExcitation;
    return (excitation.get() != NULL) ? excitation->getPosition() : 0U;
}

double HISSTools::IRMeasure::getNormPositionLocked()
{
    const std::auto_ptr<Excitation> & excitation = mExcitationAndBuffer.mExcitation;
    return (excitation.get() != NULL) ? excitation->getNormPosition() : 0.0;
}

bool HISSTools::IRMeasure::isDoneLocked()
{
    return getNormPositionLocked() == 1.0;
}
