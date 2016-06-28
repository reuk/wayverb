
#include <cmath>
#include <algorithm>

#include "ExcitationSignals.h"

// Excitation

void HISSTools::Excitation::generateBlock(double *const*const outs, uintptr_t numOuts, uintptr_t nSamps)
{
    // Zero outputs
    
    for (uintptr_t i = 0; i < numOuts; i++)
        memset(outs[i], 0, nSamps * sizeof(double));

    // Generate appropriate amount of signal
    
    for (uintptr_t i = 0; i < mNumOuts; i++)
    {
        intptr_t position = mPosition - (i * (mIRLength + mSignalLength));
        uintptr_t offset = 0;
        
        // Go to next channel if this channel is not ready to start
        
        if (-position >= static_cast<intptr_t> (nSamps))
            continue;
        
        intptr_t chanNSamps = nSamps;
        
        // Reset if this channel output starts in this block
        
        if (position <= 0)
        {
            internalReset();
            offset = -position;
            chanNSamps -= offset;
            position = 0;
        }
        
        // Don't exceed the signal length
        
        if (position + chanNSamps > static_cast<intptr_t> (mSignalLength))
            chanNSamps = std::max(static_cast<intptr_t> (0), static_cast<intptr_t> (mSignalLength) - position);
        
        generateN(outs[i] + offset, true, chanNSamps);
    }

    mPosition = std::min(mPosition + nSamps, getMeasurementLength());
}

void HISSTools::Excitation::generate(double *output, bool useAmp)
{
    internalReset();
    generateN(output, useAmp, mSignalLength);
}

// Exponential Sine Sweep (ESS)

HISSTools::ExponentialSineSweep::ExponentialSineSweep(uint32_t numIns, uint32_t numOuts, double freqLo, double freqHi, double fadeIn, double fadeOut, double length, double IRLength, double samplingRate, double amp, const FrequencyCurve& ampCurve)
: Excitation(IRLength, samplingRate, numIns, numOuts, amp)
{
    // N.B. - All times in seconds
   
    double f1 = freqLo / samplingRate;
    double f2 = freqHi / samplingRate;
    double T1 = length * samplingRate;
    double L = round(f1 * T1 / (log(f2 / f1))) / f1;
    
    // Sweep Time Constants
    
    mK1 = 2.0 * M_PI * f1 * L;
    mK2 = 1.0 / L;
    
    // Calculate Adjusted Length
    
    double T2 = round(f1 * T1 / (log(f2 / f1))) * (log(f2 / f1) / f1);
    double endPhase = floor(L * f1 * (exp(T2 * mK2) - 1.0));
    double T3 = ceil(log((endPhase / L / f1 + 1.0)) / mK2);
    
    // Store Parameters
    
    mF1  = f1;
    mSignalLength = static_cast<uintptr_t>(T3);
    mFreqRatio = exp(T3 / L);
    mFiN = std::max(fadeIn * samplingRate * 2.0, 1.0);
    mFoN = std::max(fadeOut * samplingRate * 2.0, 1.0);
    
    // Copy amplitude curve (transforming frequencies into a time representation)
    
    mAmpCurve = ampCurve;
    mAmpCurve.transform(freqTimeTransform(f1, samplingRate));
    
    reset();
}

uintptr_t HISSTools::ExponentialSineSweep::getHarmonicOffset(uint32_t harmonic)
{
    return mSignalLength / log(mFreqRatio) * log(static_cast<double>(harmonic));
}

void HISSTools::ExponentialSineSweep::generateInverse(double *output, bool useAmp)
{
    double amp = useAmp ? getAmp() : 1.0;
    amp = (4.0 * mF1 * mK2) / amp;
    
    // FIX - WTF - check this (and propogate fixes backward)....
    //double amp_const = (inv_amp == true) ? (4.0 / amp) * mLoFreq * mK2 : amp;
    
    mAmpCurve.end();
    
    for (uintptr_t i = 0; i < mSignalLength; i++)
    {
        // Fades and time value
        
        double fadeI = (1.0 - cos(M_PI * std::min(0.5, (mSignalLength - i - 1) / mFiN)));
        double fadeO = (1.0 - cos(M_PI * std::min(0.5, (i + 1) / mFoN)));
        double time = (mSignalLength - i - 1U) * mK2;
        
        // Final value
        
        *output++ = mAmpCurve(time) * amp * fadeI * fadeO * exp(time) * sin(mK1 * (exp(time) - 1.0));
    }
}

void HISSTools::ExponentialSineSweep::generateN(double *output, bool useAmp, uintptr_t nSamps)
{
    double amp = useAmp ? getAmp() : 1.0;
    
    for (uintptr_t i = mPosition; i < mPosition + nSamps; i++)
    {
        // Fades and time value
        
        double fadeI = (1.0 - cos(M_PI * std::min(0.5, i / mFiN)));
        double fadeO = (1.0 - cos(M_PI * std::min(0.5, (mSignalLength - i) / mFoN)));
        double time = i * mK2;
        
        // Final value
        
        *output++ = mAmpCurve(time) * amp * fadeI * fadeO * sin(mK1 * (exp(time) - 1.0));
    }
    
    mPosition += nSamps;
}

void HISSTools::ExponentialSineSweep::internalReset()
{
    mPosition = 0U;
    mAmpCurve.begin();
}

// Maximum Length Sequence (MLS)

HISSTools::MaximumLengthSequence::MaximumLengthSequence(uint32_t numIns, uint32_t numOuts, uint32_t order, double IRLength, double samplingRate, double amp)
: Excitation(IRLength, samplingRate, numIns, numOuts, amp)
{
    mOrder = std::max(1U, std::min(order, 24U));
    mSignalLength = (1U << mOrder) - 1U;
    
    reset();
}

void HISSTools::MaximumLengthSequence::generateN(double *output, bool useAmp, uintptr_t nSamps)
{
    double amp = useAmp ? getAmp() : 1.0;

    for (uintptr_t i = 0; i < nSamps; i++)
    {
        *output++ = amp * (2.0 * (mLSFR & 0x1U) - 1.0);
        mLSFR = (mLSFR >> 1U) ^ static_cast<uint32_t> ((0U - (mLSFR & 0x1U)) & MaskValues[mOrder]);
    }
}

void HISSTools::MaximumLengthSequence::internalReset()
{
    mLSFR = 0x1U;
}

const uint32_t HISSTools::MaximumLengthSequence::MaskValues[] =
{
    0x000000U,
    0x000000U,
    0x000002U,
    0x000006U,
    0x00000CU,
    0x000014U,
    0x000030U,
    0x000060U,
    0x0000E1U,
    0x000100U,
    0x000240U,
    0x000500U,
    0x000E08U,
    0x001C80U,
    0x003802U,
    0x006000U,
    0x00D008U,
    0x012000U,
    0x020400U,
    0x072000U,
    0x090000U,
    0x500000U,
    0xC00000U,
    0x420000U,
    0xE10000U
};

// Pink Noise Excitation (Pink)

// One pole

HISSTools::PinkNoiseExcitation::OnePole::OnePole(double freq, double samplingRate)
{
    // FIX - warping?
    
    alpha = sin(M_PI * 2.0 * freq / samplingRate);
    reset();
}

double HISSTools::PinkNoiseExcitation::OnePole::operator()(double x)
{
    return (y1 = y1 + (alpha * (x - y1)));
}

void HISSTools::PinkNoiseExcitation::OnePole::reset()
{
    y1 = 0.0;
}

// Random Number Generator

void HISSTools::PinkNoiseExcitation::Random::reset()
{
    w = x = y = 0U;
    z = 4294967295U;
}

double HISSTools::PinkNoiseExcitation::Random::operator()()
{
    uint32_t r = (x ^ (x << 20)) ^ (y ^ (y >> 11)) ^ (z ^ (z << 27)) ^ (w ^ (w >> 6));

    x = y;
    y = z;
    z = w;
    w = r;
    
    return r * 2.32830643653869628906e-10;
}

// Constructor and methods

HISSTools::PinkNoiseExcitation::PinkNoiseExcitation(uint32_t numIns, uint32_t numOuts, double fadeIn, double fadeOut, double length, double IRLength, double samplingRate, double amp)
: Excitation(IRLength, samplingRate, numIns, numOuts, amp)
{
    // Pinking filters
    
    mOnePoles.resize(5);
    
    mOnePoles[0] = OnePole(8.00135734209627, samplingRate);
    mOnePoles[1] = OnePole(46.88548507044182, samplingRate);
    mOnePoles[2] = OnePole(217.61558695916962, samplingRate);
    mOnePoles[3] = OnePole(939.80665948455472, samplingRate);
    mOnePoles[4] = OnePole(3276.10128392439381, samplingRate);
    
    // N.B. - All times in seconds
    
    mSignalLength = round(length * samplingRate);
    mFiN = std::max(fadeIn * samplingRate * 2.0, 1.0);
    mFoN = std::max(fadeOut * samplingRate * 2.0, 1.0);
    
    reset();
}

void HISSTools::PinkNoiseExcitation::generateN(double *output, bool useAmp, uintptr_t nSamps)
{
    double amp = useAmp ? getAmp() : 1.0;

    for (uintptr_t i = mPosition; i < mPosition + nSamps; i++)
    {
        double fadeI = (1.0 - cos(M_PI * std::min(0.5, i / mFiN)));
        double fadeO = (1.0 - cos(M_PI * std::min(0.5, (mSignalLength - i) / mFoN)));
    
        double white = (mRandom() * amp * 2.0) - amp;
        
        double pink = mOnePoles[0](white * 48.69991228070175);
        pink += mOnePoles[1](white * 11.23890718562874);
        pink += mOnePoles[2](white * 04.96296774193548);
        pink += mOnePoles[3](white * 02.32573483146067);
        pink += mOnePoles[4](white * 01.18433822222222);
        pink += (mYa1 = -0.7616 * mYa1 - white * 0.0168980);
        pink += mYb1 + white * 0.5362;
        mYb1 = white * 0.115926;
        
        // FIX - amplitude...
        
        *output++ = fadeI * fadeO * pink * 0.01;
    }
    
    mPosition += nSamps;
}

void HISSTools::PinkNoiseExcitation::internalReset()
{
    for (unsigned int i =0; i < 5; i++)
        mOnePoles[i].reset();
    
    mYa1 = mYb1 = 0.0;
    
    mPosition = 0U;
    mRandom.reset();
}
