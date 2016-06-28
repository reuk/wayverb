
#ifndef CLASS_DSJ_AURORA_AcousticParameters_
#define CLASS_DSJ_AURORA_AcousticParameters_

#include <vector>

static const double infiniteTime = std::numeric_limits<double>::infinity();

namespace HISSTools {
    
class AcousticParameters
{
	
public:
    
	AcousticParameters(double *ir, size_t size, double sampleRate);
	AcousticParameters(std::vector<double> ir, double sampleRate);
	AcousticParameters(const AcousticParameters& newAcousticParameters);
    
	size_t getSize();
	double getSampleRate();
	double getOnsetPositionTime();
	size_t getOnsetPositionSamples();
	
	void setOnsetTime(double onsetPositionTime);
	void setOnsetSamples(size_t onsetPositionSamples);
    
    double soundStrength(AcousticParameters &y)       { return decibels(energyRatio(y, 0.0, infiniteTime, 0.0, infiniteTime)); }
    
    double centreOfGravity();

    double earlyLateral(AcousticParameters &y)        { return energyRatio(y, 0.005, 0.08, 0.0, 0.08); }
    double earlyLateralCosine(AcousticParameters &y);
    
    double lateLateral(AcousticParameters &y)         { return decibels(energyRatio(y, 0.08, infiniteTime, 0.0, infiniteTime)); }
    
    double customParam(AcousticParameters &y, double xStart, double xEnd, double yStart, double yEnd)
    {
        return energyRatio(y, xStart, xEnd, yStart, yEnd);
    }

    
    double clarity(double time)         { return decibels(energyRatio(0.0, time, time, infiniteTime)); }
    double definition(double time)      { return energyRatio(0.0, time, 0.0, infiniteTime); }
    
    double earlySupport()               { return decibels(energyRatio(0.02, 0.1, 0.0, 0.01)); }
    double lateSupport()                { return decibels(energyRatio(0.1, 1.0, 0.0, 0.01)); }
    
    double directRatio()                { return decibels(energyRatio(0.0, 0.05, 0.0, infiniteTime)); }
    
    double edt()   { return samples2Time(reverbTime( 0.0, -10.0, -60.0)); }
    double t20()   { return samples2Time(reverbTime(-5.0, -25.0, -60.0)); }
    double t30()   { return samples2Time(reverbTime(-5.0, -35.0, -60.0)); }
    
    double customReverbTime(double hiDB, double loDB, double intersect)
    {
        return samples2Time(reverbTime(hiDB, loDB, intersect));
    }
    
    // Correlation
    
    double correlation(AcousticParameters& yIn, double start, double end, double lags);
    
private:

    // Calcuate temporary buffers
    
    void calcCummulativeEnergy();
    void calcDBIntegration();
    
    // Energy
    
    double energy();
    double energy(size_t start, size_t end);
    
    // Energy Ratios
    
    double energyRatio(AcousticParameters &yIn, double xStart, double xEnd, double yStart, double yEnd);
    double energyRatio(double start1, double end1, double start2, double end2);
    
    // Reverb Time Calculations
    
    void linearRegression(std::vector<double> &data, double *slope, double *offset);
    double reverbTime(double hiDB, double loDB, double intersect);
    
    // Conversions
    
    double decibels(double x);
    uint32_t time2Samples(double time);
    double samples2Time(size_t samples);
    
    template<typename T> T clip(T x, T low, T high)
    {
        if (x < low)
            x = low;
        if (x > high)
            x = high;
        
        return x;
    }
    
    // Data

    std::vector<double> mIR;
    std::vector<double> mIRCummulativeEnergy;
    std::vector<double> mIRDBIntegration;
    double mSampleRate;
    size_t mOnset;
    
};
    
}

#endif
