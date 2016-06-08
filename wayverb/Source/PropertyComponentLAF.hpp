#pragma once

#include "VisualiserLookAndFeel.hpp"

class PropertyComponentLAF : public VisualiserLookAndFeel {
public:
    PropertyComponentLAF(int label_width);
    Rectangle<int> getPropertyComponentContentPosition(
            PropertyComponent& p) override;

    int label_width;
};
