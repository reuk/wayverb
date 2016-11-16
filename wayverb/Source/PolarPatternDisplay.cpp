#include "PolarPatternDisplay.h"

PolarPatternView::PolarPatternView() {
    set_help("polar pattern",
             "Shows the polar pattern corresponding to the selected shape "
             "parameter");
}

void PolarPatternView::paint(Graphics& g) {
    Path p;

    auto o = getLocalBounds().getCentre();
    auto base_rad = getHeight() * 0.4;
    auto segments = 50;
    for (auto i = 0; i != segments; ++i) {
        auto angle = i * M_PI * 2 / (segments - 1);
        auto rad = std::abs(
                base_rad * ((1 - shape_) + shape_ * std::cos(angle)));

        if (i == 0) {
            p.startNewSubPath(o.x + rad * std::sin(angle),
                              o.y - rad * std::cos(angle));
        } else {
            p.lineTo(o.x + rad * std::sin(angle), o.y - rad * std::cos(angle));
        }
    }
    g.setColour(Colours::lightgrey);
    g.strokePath(p, PathStrokeType(2.0f));
}

void PolarPatternView::set_shape(double shape) {
    shape_ = shape;
    repaint();
}

////////////////////////////////////////////////////////////////////////////////

PolarPatternProperty::PolarPatternProperty(const String& name, int height)
        : PropertyComponent{name, height} {
    addAndMakeVisible(view_);
}
    
void PolarPatternProperty::refresh() {}

void PolarPatternProperty::set_shape(double shape) {
    view_.set_shape(shape);
}
