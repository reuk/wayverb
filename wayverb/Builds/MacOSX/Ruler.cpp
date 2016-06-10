#include "Ruler.hpp"

#include "lerp.h"

#include "VisualiserLookAndFeel.hpp"

#include "glm/glm.hpp"

#include <iomanip>
#include <sstream>

Ruler::Ruler() = default;
Ruler::~Ruler() noexcept = default;

void Ruler::set_max_range(const Range<float>& r) {
    assert(r.getLength() > 0);
    max_range = r;
    repaint();
}

void Ruler::set_visible_range(const Range<float>& r, bool notify) {
    assert(r.getLength() > 0);
    visible_range = r;
    repaint();

    if (notify) {
        listener_list.call(&Listener::ruler_visible_range_changed, this, r);
    }
}

void Ruler::paint(Graphics& g) {
    assert(max_range.contains(visible_range));

    VisualiserLookAndFeel::matte_foreground_box(
            g, 0, 0, getWidth(), getHeight(), Colours::darkgrey);

    g.setFillType(FillType(ColourGradient(
            Colours::white, 0, 0, Colours::lightgrey, 0, getHeight(), false)));

    auto minDivision = 30;
    auto prec = std::ceil(
            std::log10((visible_range.getLength() * minDivision) / getWidth()));
    auto s = std::pow(10, prec);

    float begin = std::floor(visible_range.getStart() / s) * s;

    g.setColour(Colours::lightgrey);
    for (auto i = begin; i < visible_range.getEnd(); i += s) {
        auto pos = lerp(i, visible_range, Range<float>(0, getWidth()));
        g.drawVerticalLine(pos, 1, getHeight() - 2);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(std::max(0.0f, -prec)) << i;
        g.setFont(10);
        g.drawText(ss.str(),
                   pos + 2,
                   0,
                   s * getWidth() / visible_range.getLength(),
                   getHeight(),
                   Justification::left);
    }
}

float Ruler::get_time(float x) const {
    return lerp(x, Range<float>(0, getWidth()), visible_range);
}

struct Ruler::RulerState {
    RulerState(float mouse_down_time, Range<float> visible_range)
            : mouse_down_time(mouse_down_time)
            , visible_range(visible_range) {
    }
    float mouse_down_time;
    Range<float> visible_range;
};

void Ruler::mouseEnter(const MouseEvent& event) {
    setMouseCursor(MouseCursor::LeftRightResizeCursor);
}

void Ruler::mouseDown(const MouseEvent& e) {
    ruler_state = std::make_unique<RulerState>(get_time(e.getMouseDownX()),
                                               visible_range);
    e.source.enableUnboundedMouseMovement(true, false);
}

void Ruler::mouseUp(const MouseEvent& event) {
    ruler_state = nullptr;
}

void Ruler::mouseExit(const MouseEvent& event) {
    setMouseCursor(MouseCursor::NormalCursor);
}

void Ruler::mouseDrag(const MouseEvent& e) {
    assert(ruler_state);

    auto dy = -e.getDistanceFromDragStartY();

    auto doubleDist = 100.0;
    auto scale = pow(2.0, dy / doubleDist);

    float w = ruler_state->visible_range.getLength() * scale;

    //  the length of the time range to display
    auto clamped = glm::clamp(w, 0.001f, max_range.getLength());

    //  move range so that ruler_state->mouse_down_time is at x
    auto x = e.getPosition().x;

    auto left = (0 - x) * clamped / getWidth() + ruler_state->mouse_down_time;
    auto right = (getWidth() - x) * clamped / getWidth() +
                 ruler_state->mouse_down_time;

    auto desired_range = Range<float>(left, right);

    set_visible_range(max_range.constrainRange(desired_range), true);
}

void Ruler::mouseDoubleClick(const MouseEvent& event) {
    set_visible_range(max_range, true);
}

void Ruler::addListener(Listener* listener) {
    listener_list.add(listener);
}
void Ruler::removeListener(Listener* listener) {
    listener_list.remove(listener);
}
