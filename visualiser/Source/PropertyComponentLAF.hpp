#pragma once

#include "VisualiserLookAndFeel.hpp"

class PropertyComponentLAF : public VisualiserLookAndFeel {
public:
    Rectangle<int> getPropertyComponentContentPosition(
        PropertyComponent& p) override;
};
