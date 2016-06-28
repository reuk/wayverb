

#ifndef _HISSTOOLS_PLOT_
#define _HISSTOOLS_PLOT_

#include <stdint.h>
#include <vector>
#include <algorithm>
#include <cmath>

namespace HISSTools
{
    
    template <class Curve, class ColorSpecification, class Renderer>
    class Plot
    {
    public:
     
        class Scaling
        {
            
        public:
            
            Scaling() : mXMin(0.0), mXMax(1.0), mYMin(0.0), mYMax(1.0), mXLog(false), mYLog(false) {}
            
            double xValToPos(double value)      { return mX + valueToPosition(value, mXLo, mXHi, mW, mXLog); }
            double yValToPos(double value)      { return mH + mY - valueToPosition(value, mYLo, mYHi, mH, mYLog); }
            double posToXVal(double position)   { return positionToValue(position - mX, mXLo, mXHi, mW, mXLog); }
            double posToYVal(double position)   { return positionToValue(mH + mY - position, mYLo, mYHi, mH, mYLog); }
            
            double getX()       { return mX; }
            double getY()       { return mY; }
            double getL()       { return getX(); }
            double getT()       { return getY(); }
            double getR()       { return getX() + getWidth(); }
            double getB()       { return getY() + getHeight(); }
            double getWidth()   { return mW; }
            double getHeight()  { return mH; }
            double getXMin()    { return mXMin; }
            double getXMax()    { return mXMax; }
            double getYMin()    { return mYMin; }
            double getYMax()    { return mYMax; }

            bool inX(double x)  {  return (x > getL() && x < getR()); }
            bool inY(double y)  {  return (y > getT() && y < getB()); }
            bool clipX(double x)  {  return std::max(getL(), std::min(x, getR())); }
            bool clipY(double y)  {  return std::max(getT(), std::min(y, getB())); }
            
            void setRangeX(double xMin, double xMax, bool logarithmic)
            {
                mXMin = xMin;
                mXMax = xMax;
                mXLo = logarithmic ? log(xMin) : xMin;
                mXHi = logarithmic ? log(xMax) : xMax;
                mXLog = logarithmic;
            }
            
            void setRangeY(double yMin, double yMax, bool logarithmic)
            {
                mYMin = yMin;
                mYMax = yMax;
                mYLo = logarithmic ? log(yMin) : yMin;
                mYHi = logarithmic ? log(yMax) : yMax;
                mYLog = logarithmic;
            }
            
            void setDimensions(double x, double y, double width, double height)
            {
                mX = x;
                mY = y;
                mW = width;
                mH = height;
            }
            
        private:
            
            double valueToPosition(double value, double loIn, double hiIn, double length, bool logarithmic)
            {
                if (logarithmic)
                    value = log(value);
                
                return (length / (hiIn - loIn)) * (value - loIn);
            }
            
            double positionToValue(double value, double loIn, double hiIn, double length, bool logarithmic)
            {
                value = (value * ((hiIn - loIn) / length)) + loIn;
                
                if (logarithmic)
                    value = exp(value);
                
                return value;
            }

            // Size, input ranges and display type
            
            double mX, mY;
            double mW, mH;
            double mXMin, mXMax, mXLo, mXHi;
            double mYMin, mYMax, mYLo, mYHi;
            bool mXLog, mYLog;
        };
        
        class CurveBase : protected Scaling
        {
            public:
            
            CurveBase(double x, double y, double w, double h, double thickness, ColorSpecification colorSpec) : mCurveTK(thickness), mCurveCS(colorSpec), mDraw(false), mDirty(false)
            {
                setDimensions(x, y, w, h);
            }
            
            ~CurveBase(){}
            
        public:

            void setThickness(double thickness)
            {
                mCurveTK = thickness;
            }
            
            void setColor(ColorSpec *colorSpec)
            {
                mCurveCS = colorSpec;
            }
            
            bool setDisplay(bool draw)
            {
                if (draw != mDraw)
                {
                    mDraw = draw;
                    return true;
                }
                
                return false;
            }
            
            void setDimensions(double x, double y, double w, double h)
            {
                Scaling::setDimensions(x, y, w, h);
            }
            
        protected:
            
            // Appearance
            
            double mCurveTK;
            ColorSpecification mCurveCS;
            
            // Flags
            
            bool mDraw;
            bool mDirty;
        };

        
    public:
        
        enum GridStyle{kGridLines, kGridPlaid};

        Plot(double x, double y, double w, double h) : mGridStyle(kGridLines), mGridXRef(0.0), mGridXGap(0.1), mGridYRef(0.0), mGridYGap(0.1), mGridXLog(false), mGridYLog(false), mFrameTK(1.0), mGridTK(1.0), mTickTK(1.0)
        {
            mScale.setDimensions(x, y, w, h);
        }
        
        ~Plot()
        {
            // FIX - how do curves get created / ownership...
            
            for (typename std::vector<Curve *>::iterator it = mCurves.begin(); it != mCurves.end(); it++)
                delete (*it);
        }

    public:
        
        virtual void needsRedraw() = 0;
        
        void setDimensions(double x, double y, double w, double h)
        {
            mScale.setDimensions(x, y, w, h);
            
            for (typename std::vector<Curve *>::iterator it = mCurves.begin(); it != mCurves.end(); it++)
                (*it)->setDimensions(x, y, w, h);
            
            needsRedraw();
        }
        
        double getX()       { return mScale.getX(); }
        double getY()       { return mScale.getY(); }
        double getL()       { return getX(); }
        double getT()       { return getY(); }
        double getR()       { return getX() + getWidth(); }
        double getB()       { return getY() + getHeight(); }
        double getWidth()   { return mScale.getWidth(); }
        double getHeight()  { return mScale.getHeight(); }
        
        void setRangeX(double xMin, double xMax, bool logarithmic)
        {
            mScale.setRangeX(xMin, xMax, logarithmic);
            needsRedraw();
        }
        
        void setRangeY(double yMin, double yMax, bool logarithmic)
        {
            mScale.setRangeY(yMin, yMax, logarithmic);
            needsRedraw();
        }
        
        void setGridX(double reference, double gap, bool logarithmic)
        {
            mGridXRef = reference;
            mGridXGap = gap;
            mGridXLog = logarithmic;
            
            needsRedraw();
        }
        
        void setGridY(double reference, double gap, bool logarithmic)
        {
            mGridYRef = reference;
            mGridYGap = gap;
            mGridYLog = logarithmic;
            
            needsRedraw();
        }
        
        void draw(Renderer *renderer)
        {
            drawBackground(renderer);
            
            for (typename std::vector<Curve *>::iterator it = mCurves.begin(); it != mCurves.end(); it++)
                (*it)->draw(renderer, mScale.getXMin(),  mScale.getXMax(),  mScale.getYMin(),  mScale.getYMax());
        }
        
        void setGridStyle(GridStyle style)
        {
            mGridStyle = style;
            needsRedraw();
        }
        
        void setCurveDisplay(bool display, unsigned long curve = 0)
        {
            if (mCurves[curve]->setDisplay(display) == true)
                needsRedraw();
        }
        
        void setCurveThickness(double thickness, unsigned long curve = 0)
        {
            mCurves[curve]->setThickness(thickness);
            needsRedraw();
        }
        
        void setCurveColor(ColorSpecification color, unsigned long curve = 0)
        {
            mCurves[curve]->setColor(color);
            needsRedraw();
        }

    private:
        
        double prevGrid(double grid, double gap, bool logarithmic)
        {
            return logarithmic ? grid / gap : grid - gap;
        }
        
        double nextGrid(double grid, double gap, bool logarithmic)
        {
            return logarithmic ? grid * gap : grid + gap;
        }
        
        double prevGridX(double grid)
        {
            return prevGrid(grid, mGridXGap, mGridXLog);
        }
        
        double prevGridY(double grid)
        {
            return prevGrid(grid, mGridYGap, mGridYLog);
        }
        
        double nextGridX(double grid)
        {
            return nextGrid(grid, mGridXGap, mGridXLog);
        }
        
        double nextGridY(double grid)
        {
            return nextGrid(grid, mGridYGap, mGridYLog);
        }
        
        double lineCentre(Renderer *renderer, double value)
        {
            return (round(value * renderer->getScaling()) + 0.5) / renderer->getScaling();
        }
        
        void horizontalTick(Renderer *renderer, double y, bool flip, double length)
        {
            // FIX - Address this
            
            /*
             if (mScale.inY(y))
             {
                y = lineCentre(renderer, y);
                double x = (flip == TRUE) ? mX - length : mX + mW;
             
             renderer->line(x, y, x + length, y, mTickTK);
             }
             */
        }
        
        void verticalTick(Renderer *renderer, double x, bool flip, double length)
        {
            // FIX - Address this
            
            /*
             if (mScale.inX(y))
             {
                x = lineCentre(renderer, x);
                double y = (flip == TRUE) ? mY - length : mY + mH;
             
                renderer->line(x, y, x, y + length, mTickTK);
             }
             */
        }
        
        void horizontalLine(Renderer *renderer, double y)
        {
            y = lineCentre(renderer, y);
            
            if (mScale.inY(y))
                renderer->line(mScale.getX(), y, mScale.getX() + mScale.getWidth(), y, mGridTK);
        }
        
        void verticalLine(Renderer *renderer, double x)
        {
            x = lineCentre(renderer, x);

            if (mScale.inX(x))
                renderer->line(x, mScale.getY(), x, mScale.getY() + mScale.getHeight(), mGridTK);
        }
        
        void horizontalBox(Renderer *renderer, double y1, double y2)
        {
            y1 = mScale.clipY(y1);
            y2 = mScale.clipY(y2);
            
            if (y1 != y2)
                renderer->fillRect(mScale.getX(), y1, mScale.getWidth(), y2 - y1);
        }
        
        void verticalBox(Renderer *renderer, double x1, double x2)
        {
            x1 = mScale.clipX(x1);
            x2 = mScale.clipX(x2);
            
            if (x1 != x2)
                renderer->fillRect(x1, mScale.getY(), x2 - x1, mScale.getHeight());
        }
        
        void drawBackground(Renderer *renderer)
        {
            double l = mScale.getL();
            double r = mScale.getR();
            double t = mScale.getT();
            double b = mScale.getB();
            double x, y, grid;
            
            // Background Color
            
            renderer->setColor(mBackgroundCS);
            renderer->fillRect(mScale.getX(), mScale.getY(), mScale.getWidth(), mScale.getHeight());
      
            // Grid
            
            renderer->setColor(mGridCS);
            
            switch (mGridStyle)
            {
                case kGridLines:
                    
                    for (x = mScale.xValToPos(grid = mGridXRef); x < r; x = mScale.xValToPos(grid = nextGridX(grid)))
                        verticalLine(renderer, x);
                    for (x = mScale.xValToPos(grid = prevGridX(mGridXRef)); x > l; x = mScale.xValToPos(grid = prevGridX(grid)))
                        verticalLine(renderer, x);
                    for (y = mScale.yValToPos(grid = mGridYRef); y > t; y = mScale.yValToPos(grid = nextGridY(grid)))
                        horizontalLine(renderer, y);
                    for (y = mScale.yValToPos(grid = prevGridY(mGridYRef)); y < b; y = mScale.yValToPos(grid = prevGridY(grid)))
                        horizontalLine(renderer, y);
                    break;
                    
                case kGridPlaid:
                
                    for (x = mScale.xValToPos(grid = mGridXRef); x < r; x = mScale.xValToPos(grid = nextGridX(nextGridX(grid))))
                        verticalBox(renderer, x, mScale.xValToPos(nextGridX(grid)));
                    for (x = mScale.xValToPos(grid = prevGridX(mGridXRef)); x > l; x = mScale.xValToPos(grid = prevGridX(prevGridX(grid))))
                        verticalBox(renderer, mScale.xValToPos(prevGridX(mGridXRef)), x);
                    for (y = mScale.yValToPos(grid = mGridYRef); y > t; y = mScale.yValToPos(grid = nextGridY(nextGridY(grid))))
                        horizontalBox(renderer, y, mScale.yValToPos(nextGridY(grid)));
                    for (y = mScale.yValToPos(grid = prevGridY(mGridYRef)); y < b; y = mScale.yValToPos(grid = prevGridY(prevGridY(grid))))
                        horizontalBox(renderer, mScale.yValToPos(prevGridY(grid)), y);
                    break;
            }
            
            renderer->setColor(mTickCS);
            
            // Ticks
            
            for (x = mScale.xValToPos(grid = mGridXRef); x < r; x = mScale.xValToPos(grid = nextGridX(grid)))
                verticalTick(renderer, x, false, 10);
            for (x = mScale.xValToPos(grid = prevGridX(mGridXRef)); x > l; x = mScale.xValToPos(grid = prevGridX(grid)))
                verticalTick(renderer, x, false, 10);
            for (y = mScale.yValToPos(grid = mGridYRef); y > t; y = mScale.yValToPos(grid = nextGridY(grid)))
                horizontalTick(renderer, y, false, 10);
            for (y = mScale.yValToPos(grid = prevGridY(mGridYRef)); y < b; y = mScale.yValToPos(grid = prevGridY(grid)))
                horizontalTick(renderer, y, false, 10);
            
            // Frame
            
            renderer->setColor(mFrameCS);
            renderer->frameRect(mScale.getX(), mScale.getY(), mScale.getWidth(), mScale.getHeight(), mFrameTK);
        }
        
    protected:
        
        // Colors

        ColorSpecification mBackgroundCS, mGridCS, mFrameCS, mTickCS;

        // Line Thicknesses
        
        double mGridTK, mFrameTK, mTickTK;
        
        // Curve
        
        std::vector<Curve *> mCurves;
        
    private:

        // Scaling and Dimensions
        
        Scaling mScale;

        // Grid Style
        
        GridStyle mGridStyle;
        
        // Grid Information
        
        double mGridXRef, mGridXGap, mGridYRef, mGridYGap;
        bool mGridXLog, mGridYLog;
    };
}

#endif /* defined(_HISSTOOLS_PLOT_) */
