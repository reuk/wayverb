#include "TransportDisplay.h"

#include <chrono>
#include <iomanip>
#include <memory>
#include <sstream>

TransportDisplay::TransportDisplay(
        juce::AudioTransportSource& audioTransportSource)
        : audioTransportSource(audioTransportSource) {
    label.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(label);

    startTimer(10);
}

void TransportDisplay::paint(juce::Graphics& g) {
    g.setColour(juce::Colour(0x40, 0x40, 0x40));
    g.fillRoundedRectangle(0, 2, getWidth(), getHeight() - 2, 4);

    g.setColour(juce::Colour(0x10, 0x10, 0x10));
    g.fillRoundedRectangle(0, 0, getWidth(), getHeight() - 2, 4);

    juce::uint8 rr = 0x00, gg = 0x00, bb = 0x00, aa = 0x80;
    g.setColour(juce::Colour(rr, gg, bb, aa));
    g.fillRoundedRectangle(0, 1, getWidth(), getHeight() - 2, 4);
}

void TransportDisplay::resized() { label.setBounds(getLocalBounds()); }

void TransportDisplay::timerCallback() {
    auto time = audioTransportSource.getCurrentPosition();
    std::chrono::milliseconds milli(static_cast<long long>(time * 1000));

    auto hours = std::chrono::duration_cast<std::chrono::hours>(milli);
    milli -= std::chrono::duration_cast<std::chrono::milliseconds>(hours);
    auto minutes = std::chrono::duration_cast<std::chrono::minutes>(milli);
    milli -= std::chrono::duration_cast<std::chrono::milliseconds>(minutes);
    auto seconds = std::chrono::duration_cast<std::chrono::seconds>(milli);
    milli -= std::chrono::duration_cast<std::chrono::milliseconds>(seconds);

    using hundredths = std::chrono::duration<long, std::centi>;
    auto hunds = std::chrono::duration_cast<hundredths>(milli);

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << hours.count() << ":";
    ss << std::setfill('0') << std::setw(2) << minutes.count() << ":";
    ss << std::setfill('0') << std::setw(2) << seconds.count() << ".";
    ss << std::setfill('0') << std::setw(2) << hunds.count();
    label.setText(ss.str(), juce::dontSendNotification);
}
