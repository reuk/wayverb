#include "Playhead.hpp"

void Playhead::paint(Graphics& g) {
    g.fillAll(Colours::red);
}

void Playhead::mouseEnter(const MouseEvent& e) {
    setMouseCursor(MouseCursor::LeftRightResizeCursor);
}

void Playhead::mouseExit(const MouseEvent& e) {
    setMouseCursor(MouseCursor::NormalCursor);
}