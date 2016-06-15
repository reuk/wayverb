
#ifndef _HISSTOOLS_CONVOLVER_
#define _HISSTOOLS_CONVOLVER_

#include <stdint.h>
#include <vector>

#include <AH_Headers/AH_Generic_Memory_Swap.h>
#include "NToMonoConvolve.h"
#include "convolve_errors.h"

namespace HISSTools
{
    namespace DSP
    {
        class Convolver
        {

        public:

            enum LatencyMode {kLatencyZero, kLatencyShort, kLatencyMedium};

            // FIX - check if these constuctors are ambiguous - I supect they are, so imagine another way....

            Convolver(uint32_t numIns, uint32_t numOuts, LatencyMode latency);
            Convolver(uint32_t numIO, LatencyMode latency);

            ~Convolver();

            // Clear IRs

            void clear(bool resize);
            void clear(uint32_t inChan, uint32_t outChan, bool resize);

            // DSP Engine Reset

            void reset();
            t_convolve_error reset(uint32_t in_chan, uint32_t out_chan);

            // Resize and set IR

            t_convolve_error resize(uint32_t inChan, uint32_t outChan, uintptr_t impulseLength);

            t_convolve_error set(uint32_t inChan, uint32_t outChan, const float *input, uintptr_t length, bool resize);
            t_convolve_error set(uint32_t inChan, uint32_t outChan, const double *input, uintptr_t length, bool resize);

            // DSP

            void process(double **ins, double **outs, uint32_t numIns, uint32_t numOuts, uintptr_t numSamples);
            void process(std::vector<double *>& ins, std::vector<double *>outs, uintptr_t numSamples);

        private:

            void tempSetup(void *memPointer, AH_UIntPtr maxFrameSize);

            void setMXCSR();
            void resetMXCSR();

            // Data

            AH_UIntPtr mNumIns;
            AH_UIntPtr mNumOuts;
            bool mN2M;

            std::vector<float *> mInTemps;
            float *mTemp1;
            float *mTemp2;

            t_memory_swap mTemporaryMemory;

            std::vector<NToMonoConvolve *> mConvolvers;

            unsigned int mOldMXCSR;
        };
    }
}

#endif
