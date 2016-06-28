

#ifndef _SPECTRUMPLOT_
#define _SPECTRUMPLOT_

#include "PowerSpectrum.hpp"
#include "Plot.h"

namespace HISSTools
{
    template <class ColorSpecification, class Renderer>
    class SpectralCurve : public Plot<SpectralCurve<ColorSpecification, Renderer>, ColorSpecification, Renderer>::CurveBase
    {
        
    public:
        
        SpectralCurve(double x, double y, double w, double h, double thickness, ColorSpecification colorSpec, unsigned long maxFFTSize) : Plot<SpectralCurve<ColorSpecification, Renderer>, ColorSpecification, Renderer>::CurveBase(x, y, w, h, thickness, colorSpec), mNormaliseTime(false)
        {
            mSpectrum = new PowerSpectrum(maxFFTSize);
        }
        
        ~SpectralCurve()
        {
            delete mSpectrum;
        }
        
    private:
        
        double binToX(double bin)
        {
            return this->xValToPos(bin * mBinScale);
        }
        
        double powToY(double pow)
        {
            // N.B. - good down to -520dB
            
            return this->yValToPos(std::max(pow * mPowScale, 0.0000000000001));
        }
        
        long begBin(double freqMin)
        {
            long maxBin = mSpectrum->getMaxBin();
            long beg = floor(mSpectrum->getFFTSize() * freqMin / mSpectrum->getSamplingRate());
            
            return (beg < 1) ? 1 : (beg > maxBin) ? maxBin : beg;
            
        }
        
        long endBin(double freqMax)
        {
            long maxBin = mSpectrum->getMaxBin();
            long end = ceil(mSpectrum->getFFTSize() * freqMax / mSpectrum->getSamplingRate());
            
            return (end < 1) ? 1 : (end > maxBin) ? maxBin : end;
        }
        
        void scalingSetup(double freqMin, double freqMax, double dbMin, double dbMax)
        {
            mBinScale = mSpectrum->getSamplingRate() / mSpectrum->getFFTSize();
            mPowScale = mNormaliseTime ? 1.0 / mSpectrum->getFFTSize() : 1.0;
            
            this->setRangeX(freqMin, freqMax, true);
            this->setRangeY(pow(10.0, dbMin / 10.0), pow(10.0, dbMax / 10.0), true);
        }
        
    public:

        void draw(Renderer *renderer, double freqMin, double freqMax, double dbMin, double dbMax, double subSampleRender = 1.0)
        {
            double minPointX, minPointY, maxPointX, maxPointY, X, lastX;
            double *spectrum = mSpectrum->getSpectrum();
            long i;
            
            if (!this->mDraw)
                return;
            
            if (mSpectrum->getFFTSize() > 0)
            {
                // Set Scaling Constants
                
                scalingSetup(freqMin, freqMax, dbMin, dbMax);
                
                long beg = begBin(freqMin);
                long end = endBin(freqMax);

                // Draw Curve

                renderer->setClip(this->getX(), this->getY(), this->getR(), this->getB());
                renderer->setColor(this->mCurveCS);
                
                if (binToX(beg) > this->getX())
                {
                    renderer->startMultiLine(this->getX(), powToY(spectrum[beg]), this->mCurveTK);
                    renderer->continueMultiLine(binToX(beg), powToY(spectrum[beg]));
                }
                else
                     renderer->startMultiLine(binToX(beg), powToY(spectrum[beg]), this->mCurveTK);
                
                for (i = beg + 1; i < end; i++)
                {
                    lastX = renderer->getX();
                    renderer->continueMultiLine(binToX(i), powToY(spectrum[i]));
                    
                    if ((renderer->getX() - lastX) < subSampleRender)
                        break;
                }
                
                for (X = renderer->getX(); i < end; )
                {
                    minPointX = maxPointX = lastX = X;
                    
                    minPointY = spectrum[i];
                    maxPointY = spectrum[i];
                    
                    X = binToX(++i);
                    
                    while (((X - lastX) < subSampleRender) && i < end)
                    {
                        if (spectrum[i] < minPointY)
                        {
                            minPointX = X;
                            minPointY = spectrum[i];
                        }
                        if (spectrum[i] > maxPointY)
                        {
                            maxPointX = X;
                            maxPointY = spectrum[i];
                        }
                        X = binToX(++i);
                    }
                    
                    if (minPointX < maxPointX)
                    {
                        renderer->continueMultiLine(minPointX, powToY(minPointY));
                        renderer->continueMultiLine(maxPointX, powToY(maxPointY));
                    }
                    else
                    {
                        renderer->continueMultiLine(maxPointX, powToY(maxPointY));
                        renderer->continueMultiLine(minPointX, powToY(minPointY));
                    }	
                }
                
                renderer->finishMultiLine();
                renderer->setClip();
            }
        }
        
        void normaliseTime(bool normalise)
        {
            mNormaliseTime = normalise;
        }
        
        bool inputSpectrum(PowerSpectrum *inSpectrum)
        {
            if (mSpectrum->copy(inSpectrum) == true)
                return true;
        
            return false;
        }
        
    protected:
        
        // Curve Spectrum
        
        PowerSpectrum *mSpectrum;
        
    private:
        
        // Scaling
        
        bool mNormaliseTime;
        
        double mBinScale;
        double mPowScale;
    };


    template <class ColorSpecification, class Renderer>
    class SpectrumPlot : public Plot<SpectralCurve<ColorSpecification, Renderer>, ColorSpecification, Renderer>
    {
        
    public:
        
        SpectrumPlot(double x, double y, double w, double h)
        : Plot<SpectralCurve<ColorSpecification, Renderer>, ColorSpecification, Renderer>(x, y, w, h) {}
        
        void addCurve(double thickness, ColorSpecification color, unsigned long maxFFTSize)
        {
            this->mCurves.push_back(new SpectralCurve<ColorSpecification, Renderer>(this->getX(), this->getY(), this->getWidth(), this->getHeight(), thickness, color, maxFFTSize));
        }
        
        void normaliseTime(bool normalise, unsigned long curve = 0)
        {
            this->mCurves[curve]->normaliseTime(normalise);
            this->needsRedraw();
        }
        
        void inputSpectrum(PowerSpectrum *inSpectrum, unsigned long curve = 0)
        {
            if (this->mCurves[curve]->inputSpectrum(inSpectrum) == true)
                this->needsRedraw();
        }
        
    protected:
        
        // Appearance
        
        double mSubSampleRender;
    };
}

#endif /* defined(_SPECTRUMPLOT_) */
