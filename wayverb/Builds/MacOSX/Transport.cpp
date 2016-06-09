#include "Transport.hpp"

#include "CommandIDs.h"
#include "Main.hpp"

Transport::Transport(AudioTransportSource& transportSource)
        : transportSource(transportSource)
        , transportDisplay(transportSource)
        , rewindButton("", DrawableButton::ImageFitted)
        , playButton("", DrawableButton::ImageFitted) {
    transportSource.addChangeListener(this);

    auto strokeWeight = 4;
    DrawableImage rewindDI;
    {
        Image rewindImage(Image::RGB, 40, 40, true);

        Graphics g(rewindImage);
        g.setFillType(FillType(ColourGradient(
                Colours::white, 0, 0, Colours::lightgrey, 0, 40, false)));

        Path p;
        p.addTriangle(40, 0, 40, 40, 0, 20);
        p.addLineSegment(Line<float>(0, 0, 0, 40), strokeWeight);

        g.fillPath(p);

        rewindDI.setImage(rewindImage);
    }
    rewindButton.setImages(&rewindDI);

    DrawableImage playDI;
    {
        Image playImage(Image::RGB, 40, 40, true);

        Graphics g(playImage);
        g.setFillType(FillType(ColourGradient(
                Colours::white, 0, 0, Colours::lightgrey, 0, 40, false)));

        Path p;
        p.addTriangle(0, 0, 0, 40, 40, 20);

        g.fillPath(p);

        playDI.setImage(playImage);
    }

    DrawableImage pauseDI;
    {
        Image pauseImage(Image::RGB, 40, 40, true);

        Graphics g(pauseImage);
        g.setFillType(FillType(ColourGradient(
                Colours::white, 0, 0, Colours::lightgrey, 0, 40, false)));

        Path p;
        p.addRectangle(0, 0, 16, 40);
        p.addRectangle(24, 0, 16, 40);

        g.fillPath(p);

        pauseDI.setImage(pauseImage);
    }
    playButton.setImages(&playDI, nullptr, nullptr, nullptr, &pauseDI);

    playButton.setClickingTogglesState(true);

    rewindButton.addListener(this);
    playButton.addListener(this);

    transportDisplay.setWantsKeyboardFocus(false);
    rewindButton.setWantsKeyboardFocus(false);
    playButton.setWantsKeyboardFocus(false);

    addAndMakeVisible(transportDisplay);
    addAndMakeVisible(rewindButton);
    addAndMakeVisible(playButton);
}

void Transport::resized() {
    auto padding = 8;
    auto buttonHeight = getHeight() - padding * 2;

    transportDisplay.setSize(100, buttonHeight);
    rewindButton.setSize(buttonHeight, buttonHeight);
    playButton.setSize(buttonHeight, buttonHeight);

    transportDisplay.setTopLeftPosition(padding, padding);
    rewindButton.setTopLeftPosition(transportDisplay.getRight() + padding,
                                    padding);
    playButton.setTopLeftPosition(rewindButton.getRight() + padding, padding);
}

void Transport::buttonClicked(Button* b) {
    auto& command_manager = VisualiserApplication::get_command_manager();
    if (b == &rewindButton) {
        command_manager.invokeDirectly(CommandIDs::idReturnToBeginning, false);
    } else if (b == &playButton) {
        if (b->getToggleState()) {
            command_manager.invokeDirectly(CommandIDs::idPlay, false);
        } else {
            command_manager.invokeDirectly(CommandIDs::idPause, false);
        }
    }
}

void Transport::changeListenerCallback(ChangeBroadcaster* source) {
    if (source == &transportSource) {
        playButton.setToggleState(transportSource.isPlaying(),
                                  dontSendNotification);
        auto& command_manager = VisualiserApplication::get_command_manager();
        command_manager.invokeDirectly(CommandIDs::idReturnToBeginning, false);
    }
}