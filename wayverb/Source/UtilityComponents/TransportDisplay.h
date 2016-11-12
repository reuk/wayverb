#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <memory>

class TransportDisplay : public juce::Component, public juce::Timer {
public:
    TransportDisplay(juce::AudioTransportSource& audioTransportSource);
    virtual ~TransportDisplay() noexcept = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void timerCallback() override;

private:
    juce::AudioTransportSource& audioTransportSource;

    juce::Label label;
};
