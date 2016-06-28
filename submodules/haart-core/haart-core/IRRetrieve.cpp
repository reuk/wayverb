
#include "ExcitationSignals.h"
#include "IRRetrieve.h"
#include "HIRT_Core.hpp"

#include <cmath>
#include <algorithm>


HISSTools::IRRetrieve::IRRetrieve(Excitation *excitation, double *recording, DeconvolutionSpecification specification) : mExcitation(excitation), mRecording(recording)
{
    double samplingRate = excitation->getSamplingRate();
    uintptr_t recLength = excitation->getMeasurementLength();
    uintptr_t genLength = excitation->getSignalLength();
    
    // FIX - must catch memory exceptions throughout
    
    HIRT_Core IRProcessor(recLength + genLength);
    
    mFFTSize = IRProcessor.nextPowerOf2(recLength + genLength);
    
    // Check and calculate lengths
    
    // FIX - this use of vector is C+11
    // 'C'+11 == 78
    
    std::vector<double> excitationSignal = std::vector<double>(genLength);

    mOutputMemory = new double[mFFTSize * excitation->getNumIns()];
    
    Spectrum spectrum1 = Spectrum(mFFTSize);
    Spectrum spectrum2 = Spectrum(mFFTSize);
    Spectrum spectrum3 = Spectrum(mFFTSize);
    
    // Generate Signal and transform into spectrum 2
    
    excitation->generate(excitationSignal.data(), false);
    IRProcessor.timeToSpectrum(excitationSignal.data(), &spectrum2, genLength, mFFTSize, samplingRate);
    
    // FIX - options to invert amplitude...

    if (excitation->hasInverse())
    {
        // Calculate standard filter for bandlimited deconvolution (sweep * inv sweep) and convert phase
        
        excitation->generateInverse(excitationSignal.data(), false);
        
        IRProcessor.timeToSpectrum(excitationSignal.data(), &spectrum3, genLength, mFFTSize, samplingRate);
        IRProcessor.convolve(&spectrum3, &spectrum2);
        // FIX - is this the best way to pass this?
        IRProcessor.changePhase(&spectrum3, specification.getDeconvolvePhase(), true);
    }
    else
    {        
        FFT_SPLIT_COMPLEX_D tempSpectrum = *(spectrum2.getSpectrum());
        
        double maxPower = 0.0;
        double testPower;
        
        // Find maximum power to scale
        
        for (uintptr_t i = 1; i < (mFFTSize >> 1); i++)
        {
            testPower = tempSpectrum.realp[i] * tempSpectrum.realp[i] + tempSpectrum.imagp[i] * tempSpectrum.imagp[i];
            maxPower = testPower > maxPower ? testPower : maxPower;
        }
        
        IRProcessor.setDeconvolutionSpecification(specification);
        IRProcessor.setFilterSpecOffset(IRProcessor.powToDb(maxPower));
        IRProcessor.makeDeconvolutionFilter(&spectrum2, &spectrum3);
    }
    
    // Deconvolve each input channel
    
    for (unsigned long i = 0; i < excitation->getNumIns(); i++)
    {
        // Do transform into spectrum1 for measurement recording - deconvolve - transform back
        
        IRProcessor.timeToSpectrum(recording + (i * recLength), &spectrum1, recLength, mFFTSize, samplingRate);
        IRProcessor.deconvolveWithFilter(&spectrum1, &spectrum2, &spectrum3);
        IRProcessor.spectrumToTime(mOutputMemory + (i * mFFTSize), &spectrum1);
    }
}

HISSTools::IRRetrieve::~IRRetrieve()
{
    delete mExcitation;
    delete[] mRecording;
    delete[] mOutputMemory;
}

// FIX - reference counted and garbage collected pointers?...  HISSTools_RefPtr <double>  or something in STL??

const double *HISSTools::IRRetrieve::getIR(uintptr_t& length, uintptr_t inChan, uintptr_t outChan, uint32_t harmonic) const
{
    if (inChan < mExcitation->getNumIns() && outChan < mExcitation->getNumOuts() && ((harmonic == 1) || mExcitation->separatesHarmonics()))
    {
        uintptr_t offset = outChan * (mExcitation->getIRLength() + mExcitation->getSignalLength());
        
        if (harmonic > 1)
        {
            uintptr_t timeHarm = mExcitation->getHarmonicOffset(harmonic);
            uintptr_t timePrev = mExcitation->getHarmonicOffset(harmonic - 1);
            uintptr_t harmLength = timeHarm - timePrev;
            
            offset = mFFTSize - timeHarm;
            length = std::min(harmLength, mExcitation->getIRLength());
        }
        else
            length = mExcitation->getIRLength();
        
        return mOutputMemory + inChan * length + offset;
    }
    
    return mOutputMemory;
}

const double *HISSTools::IRRetrieve::getDump(uintptr_t& length, uintptr_t inChan) const
{
    if (inChan < mExcitation->getNumIns())
    {
        length = mFFTSize;
        return mOutputMemory + inChan * length;
    }
    
    length = 0;
    return NULL;
}

const double *HISSTools::IRRetrieve::getRecording(uintptr_t& length, uintptr_t inChan) const
{
    if (inChan < mExcitation->getNumIns())
    {
        length = mExcitation->getMeasurementLength();
        return mRecording + inChan * length;
    }
    
    length = 0;
    return NULL;
}
 
// Deconvolution Parameters
