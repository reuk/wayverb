#include "Transport.hpp"

Transport::Transport(juce::AudioTransportSource &transport_source)
        : transport_source(transport_source)
        , transport_display(transport_source)
        , rewind_button("", juce::DrawableButton::ImageFitted)
        , play_button("", juce::DrawableButton::ImageFitted) {
    transport_source.addChangeListener(this);

    auto strokeWeight = 4;
    juce::DrawableImage rewindDI;
    {
        juce::Image rewindImage(juce::Image::RGB, 40, 40, true);

        juce::Graphics g(rewindImage);
        g.setFillType(
                juce::FillType(juce::ColourGradient(juce::Colours::white,
                                                    0,
                                                    0,
                                                    juce::Colours::lightgrey,
                                                    0,
                                                    40,
                                                    false)));

        juce::Path p;
        p.addTriangle(40, 0, 40, 40, 0, 20);
        p.addLineSegment(juce::Line<float>(0, 0, 0, 40), strokeWeight);

        g.fillPath(p);

        rewindDI.setImage(rewindImage);
    }
    rewind_button.setImages(&rewindDI);

    juce::DrawableImage playDI;
    {
        juce::Image playImage(juce::Image::RGB, 40, 40, true);

        juce::Graphics g(playImage);
        g.setFillType(
                juce::FillType(juce::ColourGradient(juce::Colours::white,
                                                    0,
                                                    0,
                                                    juce::Colours::lightgrey,
                                                    0,
                                                    40,
                                                    false)));

        juce::Path p;
        p.addTriangle(0, 0, 0, 40, 40, 20);

        g.fillPath(p);

        playDI.setImage(playImage);
    }

    juce::DrawableImage pauseDI;
    {
        juce::Image pauseImage(juce::Image::RGB, 40, 40, true);

        juce::Graphics g(pauseImage);
        g.setFillType(
                juce::FillType(juce::ColourGradient(juce::Colours::white,
                                                    0,
                                                    0,
                                                    juce::Colours::lightgrey,
                                                    0,
                                                    40,
                                                    false)));

        juce::Path p;
        p.addRectangle(0, 0, 16, 40);
        p.addRectangle(24, 0, 16, 40);

        g.fillPath(p);

        pauseDI.setImage(pauseImage);
    }
    play_button.setImages(&playDI, nullptr, nullptr, nullptr, &pauseDI);

    play_button.setClickingTogglesState(true);
    play_button.setColour(juce::DrawableButton::backgroundColourId,
                          juce::Colours::transparentBlack);
    play_button.setColour(juce::DrawableButton::backgroundOnColourId,
                          juce::Colours::transparentBlack);

    rewind_button.addListener(this);
    play_button.addListener(this);

    transport_display.setWantsKeyboardFocus(false);
    rewind_button.setWantsKeyboardFocus(false);
    play_button.setWantsKeyboardFocus(false);

    addAndMakeVisible(transport_display);
    addAndMakeVisible(rewind_button);
    addAndMakeVisible(play_button);
}

void Transport::resized() {
    auto padding = 8;
    auto buttonHeight = getHeight() - padding * 2;

    transport_display.setSize(100, buttonHeight);
    rewind_button.setSize(buttonHeight, buttonHeight);
    play_button.setSize(buttonHeight, buttonHeight);

    transport_display.setTopLeftPosition(padding, padding);
    rewind_button.setTopLeftPosition(transport_display.getRight() + padding,
                                     padding);
    play_button.setTopLeftPosition(rewind_button.getRight() + padding, padding);
}

void Transport::buttonClicked(juce::Button *b) {
    if (b == &rewind_button) {
        transport_source.setPosition(0);
    } else if (b == &play_button) {
        if (b->getToggleState()) {
            transport_source.start();
        } else {
            transport_source.stop();
        }
    }
}

void Transport::changeListenerCallback(juce::ChangeBroadcaster *source) {
    if (source == &transport_source) {
        play_button.setToggleState(transport_source.isPlaying(),
                                   juce::dontSendNotification);
        if (transport_source.hasStreamFinished()) {
            transport_source.setPosition(0);
        }
    }
}
