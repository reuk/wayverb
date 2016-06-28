
#include "AcousticParameters.h"

#include <cmath>

namespace HISSTools {

AcousticParameters::AcousticParameters(double *ir, size_t size, double sampleRate) : mSampleRate(sampleRate), mOnset(0)
{
	mIR.assign(ir, ir + size);
}

AcousticParameters::AcousticParameters(std::vector<double> ir, double sampleRate) : mIR(ir), mSampleRate(sampleRate), mOnset(0)
{}

AcousticParameters::AcousticParameters(const AcousticParameters& newAcousticParameters)
{
	mIR = newAcousticParameters.mIR;
	mIRCummulativeEnergy = newAcousticParameters.mIRCummulativeEnergy;
	mIRDBIntegration = newAcousticParameters.mIRDBIntegration;
	mSampleRate = newAcousticParameters.mSampleRate;
	mOnset = newAcousticParameters.mSampleRate;
}

size_t AcousticParameters::getSize()
{
	return mIR.size();
}
    
double AcousticParameters::getSampleRate()
{
	return mSampleRate;
}

double AcousticParameters::getOnsetPositionTime()
{
	return samples2Time(mOnset);
}

size_t AcousticParameters::getOnsetPositionSamples()
{
	return mOnset;
}

void AcousticParameters::setOnsetTime(double onsetPositionTime)
{
	setOnsetSamples(time2Samples(onsetPositionTime));
}

void AcousticParameters::setOnsetSamples(size_t onsetPositionSamples)
{
    mOnset = onsetPositionSamples < getSize() ? onsetPositionSamples : getSize() - 1;
}

double AcousticParameters::centreOfGravity()
{
    // FIX - check this in detail...
    
    double weightedSum = 0.0;
    
    size_t onset = getOnsetPositionSamples();
    size_t length = getSize() - onset;
    
    for (size_t t = 0; t < length; t++)
        weightedSum += mIR[t + onset] * mIR[t + onset] * (double)t;

    return samples2Time(weightedSum / energy());
}

double AcousticParameters::earlyLateralCosine(AcousticParameters &y)
{
    // FIX - sample rate match!!
    
    size_t xOnset = getOnsetPositionSamples();
    size_t yOnset = y.getOnsetPositionSamples();
    
    size_t xStart = time2Samples(0.005) + xOnset;
    size_t xEnd = time2Samples(0.08) + xOnset;
    size_t yEnd = time2Samples(0.08) + yOnset;
    
    xStart = clip(xStart, (size_t) 0, getSize());
    xEnd = clip(xEnd, (size_t) 0, getSize());
    yEnd = clip(yEnd, (size_t) 0, y.getSize());
    
    if ((xStart + yOnset) >= y.getSize())
        return 0.0;
    
    if ((xEnd + yOnset) >= y.getSize())
        return xEnd -= (y.getSize() - (xEnd + yOnset));
    
    double product = 0.0;
    
    for (size_t i = xStart; i < xEnd; i++)
        product += mIR[i] * y.mIR[i + yOnset] * y.mIR[i + yOnset];
    
    return product / y.energy(yOnset, yEnd);
}

// Correlation

double AcousticParameters::correlation(AcousticParameters &yIn, double start, double end, double lags)
{
    double maxR = 0.0;
    
    // FIX - sample rate match
    
    size_t startI = time2Samples(start);
    size_t endI = time2Samples(end);
    long lagI = time2Samples(lags);
    
    size_t xOnset = getOnsetPositionSamples();
    size_t yOnset = yIn.getOnsetPositionSamples();
    
    startI = clip(startI , (size_t) 0, getSize());
    startI = clip(startI, (size_t) 0, yIn.getSize());
    endI = clip(endI, (size_t) 0, getSize());
    endI = clip(endI, (size_t) 0, yIn.getSize());
    
    double den = sqrt(energy(startI + xOnset, endI) * yIn.energy(startI + yOnset, endI));
    
    for (long cLag = -lagI; cLag <= lagI; cLag++)
    {
        long xStart = startI;
        long yStart = startI - cLag;
        long yEnd = endI - cLag;
        double xy = 0.0;

        if (yStart < 0)
            xStart = -yStart;
        clip(yStart, (long) 0, (long) yIn.getSize());
        clip(yEnd, (long) 0, (long) yIn.getSize());
        
        long windowSize = yEnd - yStart;
        
        for (long t = 0; t < windowSize; t++)
            xy += mIR[t + xStart] * yIn.mIR[t + yStart];
        
        double r = xy / den;
        
        if (r > maxR)
            maxR = r;
    }
    
    return maxR;
}

// Temporary Buffers

void AcousticParameters::calcCummulativeEnergy()
{
    if (mIRCummulativeEnergy.size() != mIR.size())
    {
        mIRCummulativeEnergy.resize(mIR.size());
        
        mIRCummulativeEnergy[0] = (mIR[0] * mIR[0]);
        
        for (size_t i = 1; i < mIR.size(); i++)
            mIRCummulativeEnergy[i] = (mIR[i] * mIR[i]) + mIRCummulativeEnergy[i - 1];
    }
}

void AcousticParameters::calcDBIntegration()
{
    calcCummulativeEnergy();
    
    if (mIRDBIntegration.size() != mIR.size())
    {
        mIRDBIntegration.resize(mIR.size());
        
        // Backwards integrate and covert to DB
        
        double sum = mIRCummulativeEnergy[mIRCummulativeEnergy.size() - 1];
        double sumRecip = 1.0 / sum;
        
        for (size_t i = 0; i < mIRDBIntegration.size(); i++)
            mIRDBIntegration[i] = decibels((sum - mIRCummulativeEnergy[i]) * sumRecip);
    }
}
    
// Energy

double AcousticParameters::energy()
{
    return energy(0, mIR.size());
}
    
double AcousticParameters::energy(size_t start, size_t end)
{
    calcCummulativeEnergy();
        
    return mIRCummulativeEnergy[end] - mIRCummulativeEnergy[start];
}
    
// Energy Ratios

double AcousticParameters::energyRatio(AcousticParameters &yIn, double xStart, double xEnd, double yStart, double yEnd)
{
    // FIX - sample rate match
    
    size_t xStartInt = time2Samples(xStart) + getOnsetPositionSamples();
    size_t xEndInt = time2Samples(xEnd) + getOnsetPositionSamples();
    size_t yStartInt = time2Samples(yStart) + yIn.getOnsetPositionSamples();
    size_t yEndInt  = time2Samples(yEnd) + yIn.getOnsetPositionSamples();
    
    xStartInt = clip(xStartInt, (size_t) 0, getSize());
    xEndInt = clip(xEndInt, (size_t) 0, getSize());
    yStartInt = clip(yStartInt, (size_t) 0, yIn.getSize());
    yEndInt = clip(yEndInt, (size_t) 0, yIn.getSize());
    
    return energy(xStartInt, xEndInt) / yIn.energy(yStartInt, yEndInt);
}

double AcousticParameters::energyRatio(double start1, double end1, double start2, double end2)
{
    size_t xStart1Int = time2Samples(start1) + getOnsetPositionSamples();
    size_t xEnd1Int = time2Samples(end1) + getOnsetPositionSamples();
    size_t xStart2Int = time2Samples(start2) + getOnsetPositionSamples();
    size_t xEnd2Int = time2Samples(end2) + getOnsetPositionSamples();
    
    xStart1Int = clip(xStart1Int, (size_t) 0, getSize());
    xEnd1Int = clip(xEnd1Int, (size_t) 0, getSize());
    xStart2Int = clip(xStart2Int, (size_t) 0, getSize());
    xEnd2Int = clip(xEnd2Int, (size_t) 0, getSize());
    
    return energy(xStart1Int, xEnd1Int) / energy(xStart2Int, xEnd2Int);
}

// Reverb calculations

void AcousticParameters::linearRegression(std::vector<double> &data, double *slope, double *offset)
{
    double sum1 = 0.0;
    double sum2 = 0.0;
    double xmean = (data.size() - 1) / 2.;
    double ymean = 0.0;
    
    for (size_t i = 0; i < data.size(); i++)
        ymean += data[i];
    
    ymean /= (double) data.size();
    
    for (size_t i = 0; i < data.size(); i++)
        sum1 += (i - xmean) * (data[i] - ymean);
    
    for (uint32_t i = 0; i < data.size(); i++)
        sum2 += (i - xmean) * (i - xmean);
    
    *slope = sum1 / sum2;
    *offset = ymean - *slope * xmean;
}

double AcousticParameters::reverbTime(double hiDB, double loDB, double intersect)
{
    double value = 0.;
    double slope;
    double offset;
    
    uint32_t hiOffset = 0;
    uint32_t loOffset = 0;
    
    calcDBIntegration();
    
    // Find relevant points in the integration
    
    for (hiOffset = 0; value > hiDB && hiOffset < mIRDBIntegration.size(); hiOffset++)
        value = mIRDBIntegration[hiOffset];
    
    for (loOffset = hiOffset; value > loDB && loOffset < mIRDBIntegration.size(); loOffset++)
        value = mIRDBIntegration[loOffset];
    
    // Do linear regression
    
    std::vector<double> regressionData(mIRDBIntegration.begin() + hiOffset, mIRDBIntegration.begin() + loOffset);
    linearRegression(regressionData, &slope, &offset);
    
    // Return result (samples)
    
    if (slope == 0.0 || isnan(slope) || isinf(slope))
        return 1.0;
    
    return ((intersect - offset) / slope) + hiOffset;
}

// Conversions

double AcousticParameters::decibels(double x)
{
    return 10.0 * log10(x);
}

uint32_t AcousticParameters::time2Samples(double time)
{
    return (time != std::numeric_limits<double>::infinity()) ? ((uint32_t)floor(time * getSampleRate())) : std::numeric_limits<uint32_t>::max();
}

double AcousticParameters::samples2Time(size_t samples)
{
    return (double)samples / getSampleRate();
}

}
