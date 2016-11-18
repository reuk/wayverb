#include "Playhead.h"

void Playhead::paint(juce::Graphics& g) {
    g.setColour(juce::Colours::red);
    g.drawVerticalLine(getWidth() * 0.5, 0, getHeight());
}

void Playhead::mouseEnter(const juce::MouseEvent& e) {
    setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
}

void Playhead::mouseExit(const juce::MouseEvent& e) {
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void Playhead::mouseDrag(const juce::MouseEvent& e) {
    listener_list.call(&Listener::playhead_dragged, this, e);
}

void Playhead::addListener(Listener* l) { listener_list.add(l); }

void Playhead::removeListener(Listener* l) { listener_list.remove(l); }
