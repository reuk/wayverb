
#ifndef _HISSTOOLS_IRRETRIEVE_
#define _HISSTOOLS_IRRETRIEVE_

#include <stdint.h>
#include <vector>

#include "DeconvolutionSpecification.h"
#include "ExcitationSignals.h"

namespace HISSTools
{
    struct IRRetrieve
    {
        
    public:
                        
        IRRetrieve(Excitation *excitation, double *recording, DeconvolutionSpecification specification = DeconvolutionSpecification());
        ~IRRetrieve();
                
        // FIX - reference counted and garbage collected pointer...  HISSTools_RefPtr <double>  or something in STL??
            
        const double *getIR(uintptr_t& length, uintptr_t inChan, uintptr_t outChan, uint32_t harmonic = 1) const;
        const double *getDump(uintptr_t& length, uintptr_t inChan) const;
        const double *getRecording(uintptr_t& length, uintptr_t inChan) const;
            
        // FIX - might be good to retrieve info about the excitation later....
            
    private:
            
        IRRetrieve(const IRRetrieve& irr);
        IRRetrieve& operator=(const IRRetrieve& irr);

        Excitation *mExcitation;
        double *mRecording;
        double *mOutputMemory;
        uintptr_t mFFTSize;
    };
}

#endif
