
#ifndef _HISSTOOLS_IRMEASURE_
#define _HISSTOOLS_IRMEASURE_

#include <stdint.h>
#include <vector>

#include "DeconvolutionSpecification.h"
#include "ExcitationSignals.h"
#include "IRRetrieve.h"
#include "Threadsafety.h"

namespace HISSTools
{
    class IRMeasure
    {
        
    public:
        
        IRMeasure();
        ~IRMeasure();
        
        // Start/Stop
        
        void start(const Excitation &excitationSignal);
        void test(const Excitation &excitationSignal);
        void cancel();
        
        // Audio Engine Reset
        
        void reset(double samplingRate);
        
        // Excitation Position
        
        uintptr_t getPosition();
        double getNormPosition();
        bool isDone();
        
        // DSP
        
        void process(const double *const *const ins, double *const*const outs, uintptr_t numIns, uintptr_t numOuts, uintptr_t numSamples);
        void process(const std::vector<double *>& ins, const std::vector<double *>& outs, uintptr_t numSamples);
        
        // Processing and Retrieve Output
        
        void setDeconvolutionSpecification(DeconvolutionSpecification& specification)    { mDeconvolutionSpecification = specification; }
        IRRetrieve *processRecording();
        
        // Deconvolution Parameters
        
    private:
        
        struct ExcitationAndBuffer
        {
            // FIX - will call delete on array..
            
            ExcitationAndBuffer(Excitation * excitation = NULL,
                                double * recordBuffer = NULL);
            std::auto_ptr<Excitation> mExcitation;
            std::auto_ptr<double> mRecordBuffer;
        };
        
        // Swap Memory Safely

        bool swap(ExcitationAndBuffer& rhs, bool requireDone = false);

        // Internal Getters
        
        uintptr_t getPositionLocked();
        double getNormPositionLocked();
        
        bool isDoneLocked();
        
        // Data
        
        DeconvolutionSpecification mDeconvolutionSpecification;
        ExcitationAndBuffer mExcitationAndBuffer;
        SpinLock mLock;
    };
}

#endif
