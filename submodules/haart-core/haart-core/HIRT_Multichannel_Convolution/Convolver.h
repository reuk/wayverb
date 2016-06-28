#ifndef _HISSTOOLS_CONVOLVER_
#define _HISSTOOLS_CONVOLVER_

#include <stdint.h>
#include <vector>

#include "AH_Generic_Memory_Swap.h"
#include "NToMonoConvolve.h"
#include "convolve_errors.h"

namespace HISSTools
{
   class Convolver
    {

    public:
        enum LatencyMode
        {
            kLatencyZero,
            kLatencyShort,
            kLatencyMedium
        };

        Convolver(uint32_t numIns, uint32_t numOuts, LatencyMode latency);
        Convolver(uint32_t numIO, LatencyMode latency);

        virtual ~Convolver() throw();

        // Clear IRs

        void clear(bool resize);
        void clear(uint32_t inChan, uint32_t outChan, bool resize);

        // DSP Engine Reset

        void reset();
        t_convolve_error reset(uint32_t in_chan, uint32_t out_chan);

        // Resize and set IR

        t_convolve_error resize(uint32_t inChan, uint32_t outChan,
                                uintptr_t impulseLength);

        t_convolve_error set(uint32_t inChan, uint32_t outChan, const float* input,
                             uintptr_t length, bool resize);
        t_convolve_error set(uint32_t inChan, uint32_t outChan, const double* input,
                             uintptr_t length, bool resize);

        // DSP

        void process(const double** ins, double** outs, size_t numIns, size_t numOuts, size_t numSamples);
        
        // FIX - consts
        
        void process(const float** ins, float** outs, size_t numIns, size_t numOuts, size_t numSamples);
       
    private:
        void tempSetup(void* memPointer, AH_UIntPtr maxFrameSize);

        void setMXCSR();
        void resetMXCSR();

        // Data

        uint32_t mNumIns;
        uint32_t mNumOuts;
        bool mN2M;

        std::vector<float*> mInTemps;
        float* mTemp1;
        float* mTemp2;

        t_memory_swap mTemporaryMemory;

        std::vector<NToMonoConvolve*> mConvolvers;

        unsigned int mOldMXCSR;
    };
}

#endif
