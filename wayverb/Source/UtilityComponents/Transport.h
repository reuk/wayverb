#pragma once

#include "TransportDisplay.h"

class Transport : public juce::Component,
                  public juce::Button::Listener,
                  public juce::ChangeListener {
public:
    Transport(juce::AudioTransportSource &transportSource);
    virtual ~Transport() noexcept = default;

    void resized() override;

    void buttonClicked(juce::Button *b) override;
    void changeListenerCallback(juce::ChangeBroadcaster *source) override;

private:
    juce::AudioTransportSource &transport_source;

    TransportDisplay transport_display;
    juce::DrawableButton rewind_button;
    juce::DrawableButton play_button;
};
