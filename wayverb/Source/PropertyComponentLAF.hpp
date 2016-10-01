#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "OtherComponents/AngularLookAndFeel.hpp"

class PropertyComponentLAF : public AngularLookAndFeel {
public:
    PropertyComponentLAF(int label_width);
    Rectangle<int> getPropertyComponentContentPosition(
            PropertyComponent& p) override;

    int label_width;
};
