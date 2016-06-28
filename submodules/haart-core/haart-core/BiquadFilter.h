
#ifndef _HISSTOOLS_BIQUADFILTER_
#define _HISSTOOLS_BIQUADFILTER_

#include <stdint.h>
#include <vector>

namespace HISSTools
{
    class BiquadFilter
    {
        
    public:

        struct Coefficients
        {
            Coefficients(double A1 = 0.0, double A2 = 0.0, double B0 = 0.0, double B1 = 0.0, double B2 = 0.0)
            : a1(A1), a2(A2), b0(B0), b1(B1), b2(B2){}
            
            double a1, a2, b0, b1, b2;
        };
        
        BiquadFilter();
        
        // DSP Engine/Filter Reset
        
        void reset();

        // Coefficient conversion
        
        static Coefficients convertCoefficients(double a0, double a1, double a2, double b0, double b1, double b2)
        {
            return Coefficients(a1/a0, a2/a0, b0/a0, b1/a0, b2/a0);
        }
        
        // Set filter parameters and type
        
        void set(Coefficients& coeff);

        // DSP
        
        double operator()(double x);
        double operator()(double x, Coefficients& coeff);

    private:
        
        // Data
        
        Coefficients mCoeff;
        double x1, x2, y1, y2;
    };
}

#endif