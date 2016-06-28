
#ifndef _HISSTOOLS_FRACTIONALOCTAVEFILTER_
#define _HISSTOOLS_FRACTIONALOCTAVEFILTER_

#include <stdint.h>
#include <vector>

namespace HISSTools
{
    class FractionalOctaveFilter
    {
        
    public:
        
        FractionalOctaveFilter(double fraction = 1.0, double reference = 1000.0);
        ~FractionalOctaveFilter();
        
        // DSP Engine/Filter Reset

        void reset(double samplingRate);

        // Set and define bands
        
        void set(int32_t band);
        void defineBands(double fraction, double reference = 1000.0);
        
        // DSP

        double operator()(double input);
        
        void process()(double *io, uintptr_t numSamples);
        void process(const double input, double *output, uintptr_t numSamples);

        void reverseProcess(double *io, uintptr_t numSamples);
        void reverseProcess(const double input, double *output, uintptr_t numSamples);
        
        // N.B. resets before and after running
        
        void forwardsBackwards(double *io, uintptr_t numSamples);
        void forwardsBackwards(const double input, double *output, uintptr_t numSamples);
        
    private:
        
        // Data

    };
}
    
#endif