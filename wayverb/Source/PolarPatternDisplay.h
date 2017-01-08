#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class PolarPatternView : public Component, public SettableTooltipClient {
public:
    PolarPatternView();
    void paint(Graphics& g) override;

    void set_shape(double shape);

private:
    double shape_{0};
};

////////////////////////////////////////////////////////////////////////////////

class PolarPatternProperty : public PropertyComponent {
public:
    PolarPatternProperty(const String& name, int height);

    void refresh() override;

    void set_shape(double shape);

private:
    PolarPatternView view_;
};
