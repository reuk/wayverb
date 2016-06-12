#pragma once

#include "TransportDisplay.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

class Transport : public Component,
                  public Button::Listener,
                  public ChangeListener {
public:
    Transport(AudioTransportSource& transportSource);
    virtual ~Transport() noexcept = default;

    void resized() override;

    void buttonClicked(Button* b) override;

    void changeListenerCallback(ChangeBroadcaster* source) override;

private:
    const AudioTransportSource& transportSource;

    TransportDisplay transportDisplay;
    DrawableButton rewindButton;
    DrawableButton playButton;
};