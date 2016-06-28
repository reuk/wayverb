
#include "BiquadFilter.h"

namespace HISSTools
{

    BiquadFilter::BiquadFilter()
    {
        reset();
    }
        
    // DSP Engine/Filter Reset
        
    void BiquadFilter::reset()
    {
        x1 = x2 = y1 = y2 = 0.0;
    }

    // Set filter parameters and type
        
    void BiquadFilter::set(Coefficients& coeff)
    {
        mCoeff = coeff;
    }

    // DSP
        
    double BiquadFilter::operator()(double x)
    {
        double y = (x * mCoeff.b0) + (x1 * mCoeff.b1) + (x2 * mCoeff.b2) - (y1 * mCoeff.a1) - (y2 * mCoeff.a2);
        
        x2 = x1;
        x1 = x;
        y2 = y1;
        y1 = y;
        
        return y;
    }
    
    double BiquadFilter::operator()(double x, Coefficients& coeff)
    {
        set(coeff);
        return BiquadFilter::operator()(x);
    }
}