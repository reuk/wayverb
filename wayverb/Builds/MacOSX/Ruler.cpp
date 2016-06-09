#include "Ruler.hpp"

#include <iomanip>
#include <sstream>

Ruler::Ruler() {
}

void Ruler::setMaximumRange(double newMax) {
    max = newMax;
}

void Ruler::paint(Graphics& g) {
    g.setFillType(FillType(ColourGradient(
        Colours::white, 0, 0, Colours::lightgrey, 0, getHeight(), false)));

    if (max > 0) {
        auto minDivision = 30;
        auto prec = ceil(log10((max * minDivision) / getWidth()));
        auto s = pow(10, prec);

        g.setColour(Colours::lightgrey);
        for (auto i = 0.0; i < max; i += s) {
            auto pos = i * getWidth() / max;
            g.drawLine(pos, 1, pos, getHeight() - 2);
            std::stringstream ss;
            ss << std::fixed << std::setprecision(std::max(0.0, -prec)) << i;
            g.setFont(10);
            g.drawText(ss.str(),
                       pos + 2,
                       0,
                       s * getWidth() / max,
                       getHeight(),
                       Justification::left);
        }
    }
}

void Ruler::resized() {
}

void Ruler::mouseEnter(const MouseEvent& event) {
    setMouseCursor(MouseCursor::LeftRightResizeCursor);
}

void Ruler::mouseDown(const MouseEvent& event) {
    auto time = event.getMouseDownX() * max / getWidth();
    listenerList.call(&Listener::rulerMouseDown, this, event, time);
}

void Ruler::mouseUp(const MouseEvent& event) {
    listenerList.call(&Listener::rulerMouseUp, this, event);
}

void Ruler::mouseExit(const MouseEvent& event) {
    setMouseCursor(MouseCursor::NormalCursor);
}

void Ruler::mouseDrag(const MouseEvent& event) {
    listenerList.call(&Listener::rulerDragged, this, event);
}

void Ruler::mouseDoubleClick(const MouseEvent& event) {
    listenerList.call(&Listener::rulerDoubleClicked, this, event);
}

void Ruler::addListener(Listener* listener) {
    listenerList.add(listener);
}
void Ruler::removeListener(Listener* listener) {
    listenerList.remove(listener);
}
