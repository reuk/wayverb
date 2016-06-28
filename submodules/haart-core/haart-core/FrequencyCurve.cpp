
#include "FrequencyCurve.h"

HISSTools::FrequencyCurve::FrequencyCurve(const std::vector<FrequencySpecifier>& specifiers)
{
    mSpecifiers = specifiers;
    sort();
}

double HISSTools::FrequencyCurve::operator()(double frequency, bool inverse)
{
    double inversion = inverse ? -1.0 : 1.0;
    
    // If empty...
    
    if (mSpecifiers.empty() == true)
        return 1.0;
    
    // Search for the next curve position
    
    while ((mPosition != mSpecifiers.end() && frequency > mPosition->mFreq))
        mPosition++;
        
    while ((mPosition != mSpecifiers.end() && mPosition != mSpecifiers.begin() && frequency < mPosition->mFreq))
        mPosition--;
    
    // Endpoints
    
    if (mPosition == mSpecifiers.begin() && frequency < mPosition->mFreq)
        return pow(10.0, inversion * mPosition->mDB / 20.0);
    
    if (mPosition == mSpecifiers.end())
        return pow(10.0, inversion * mSpecifiers.back().mDB / 20.0);
    
    // Interpolate
    
    FrequencySpecifier now = *mPosition;
    FrequencySpecifier next = *(mPosition + 1);
    
    double interp = (frequency - now.mFreq) / (next.mFreq - now.mFreq);
    double db = now.mDB + interp * (next.mDB - now.mDB);
    
    return pow(10.0, inversion * db / 20.);
}

void HISSTools::FrequencyCurve::begin()
{
    mPosition = mSpecifiers.begin();
}

void HISSTools::FrequencyCurve::end()
{
    mPosition = mSpecifiers.end();
}

void HISSTools::FrequencyCurve::sort()
{
    std::sort(mSpecifiers.begin(), mSpecifiers.end(), FrequencySpecifier::lessThan());
}
