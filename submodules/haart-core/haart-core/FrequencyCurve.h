
#ifndef _HISSTOOLS_FREQUENCYCURVE_
#define _HISSTOOLS_FREQUENCYCURVE_

#include <vector>
#include <algorithm>
#include <cmath> 

namespace HISSTools
{
    struct FrequencySpecifier
    {
        struct lessThan
        {
            inline bool operator() (const FrequencySpecifier& spec1, const FrequencySpecifier& spec2)
            {
                return (spec1.mFreq < spec2.mFreq);
            }
        };
        
        FrequencySpecifier(double frequency = 0.0, double db = 0.0) : mFreq(frequency) , mDB(db) {}
        
        double mFreq;
        double mDB;
    };
        
    struct FrequencyCurve
    {
        FrequencyCurve(){};
        FrequencyCurve(const std::vector<FrequencySpecifier>& specifiers);
        
        double operator()(double frequency, bool inverse = false);
        void begin();
        void end();
        
        void add();
        void clear();
        
        template <class UnaryOp> void transform(UnaryOp op)
        {
            std::transform(mSpecifiers.begin(), mSpecifiers.end(), mSpecifiers.begin(), op);
            sort();
        }
        
    private:
        
        void sort();
        
        std::vector<FrequencySpecifier> mSpecifiers;
        std::vector<FrequencySpecifier>::iterator mPosition;
    };
}

#endif
