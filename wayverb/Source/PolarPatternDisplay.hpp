#pragma once

#include "HelpWindow.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

class PolarPatternView : public Component, public SettableHelpPanelClient {
public:
    PolarPatternView();
    void paint(Graphics& g) override;

    void set_shape (double shape);

private:
    double shape_{0};
};
