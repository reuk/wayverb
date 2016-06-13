#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class Playhead : public Component {
public:
    void paint(Graphics& g) override;

    void mouseEnter(const MouseEvent& e) override;
    void mouseExit(const MouseEvent& e) override;
};