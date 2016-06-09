#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <memory>

class TransportDisplay : public Component, public Timer {
public:
    TransportDisplay(AudioTransportSource& audioTransportSource);
    virtual ~TransportDisplay() noexcept = default;

    void paint(Graphics& g) override;
    void resized() override;

    void timerCallback() override;

private:
    AudioTransportSource& audioTransportSource;

    Label label;
};