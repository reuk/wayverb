#include "Ruler.hpp"

#include "lerp.h"

#include "AngularLookAndFeel.hpp"

#include "glm/glm.hpp"

#include <iomanip>
#include <sstream>

Ruler::Ruler(PlaybackViewManager& m)
        : playback_view_manager(m) {
}
Ruler::~Ruler() noexcept = default;

void Ruler::paint(Graphics& g) {
    AngularLookAndFeel::matte_foreground_box(
            g,
            Rectangle<int>(-2, 0, getWidth() + 4, getHeight()),
            Colours::darkgrey);

    g.setFillType(FillType(ColourGradient(
            Colours::white, 0, 0, Colours::lightgrey, 0, getHeight(), false)));

    auto minDivision = 30;
    auto prec = std::ceil(
            std::log10((playback_view_manager.get_visible_range().getLength() *
                        minDivision) /
                       getWidth()));
    auto s = std::pow(10, prec);

    auto begin =
            std::floor(playback_view_manager.get_visible_range().getStart() /
                       s) *
            s;

    g.setColour(Colours::lightgrey);
    for (auto i = begin; i < playback_view_manager.get_visible_range().getEnd();
         i += s) {
        auto pos = lerp(i,
                        playback_view_manager.get_visible_range(),
                        Range<double>(0, getWidth()));
        g.drawVerticalLine(pos, 2, getHeight() - 2);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(std::max(0.0, -prec)) << i;
        g.setFont(10);
        g.drawText(
                ss.str(),
                pos + 2,
                0,
                s * getWidth() /
                        playback_view_manager.get_visible_range().getLength(),
                getHeight(),
                Justification::left);
    }
}

void Ruler::resized() {
}

double Ruler::x_to_time(double x) const {
    return lerp(x,
                Range<double>(0, getWidth()),
                playback_view_manager.get_visible_range());
}

double Ruler::time_to_x(double time) const {
    return lerp(time,
                playback_view_manager.get_visible_range(),
                Range<double>(0, getWidth()));
}

struct Ruler::RulerState {
    RulerState(double mouse_down_time, Range<double> visible_range)
            : mouse_down_time(mouse_down_time)
            , visible_range(visible_range) {
    }
    double mouse_down_time;
    Range<double> visible_range;
};

void Ruler::mouseEnter(const MouseEvent& event) {
    setMouseCursor(MouseCursor::LeftRightResizeCursor);
}

void Ruler::mouseDown(const MouseEvent& e) {
    ruler_state = std::make_unique<RulerState>(
            x_to_time(e.getMouseDownX()),
            playback_view_manager.get_visible_range());
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

    auto w = ruler_state->visible_range.getLength() * scale;

    //  the length of the time range to display
    auto clamped = glm::clamp(
            w, 0.001, playback_view_manager.get_max_range().getLength());

    //  move range so that ruler_state->mouse_down_time is at x
    auto x = e.getPosition().x;

    auto left = (0 - x) * clamped / getWidth() + ruler_state->mouse_down_time;
    auto right = (getWidth() - x) * clamped / getWidth() +
                 ruler_state->mouse_down_time;

    playback_view_manager.set_visible_range(Range<double>(left, right), true);
}

void Ruler::mouseDoubleClick(const MouseEvent& event) {
    playback_view_manager.set_visible_range(
            playback_view_manager.get_max_range(), true);
}

void Ruler::max_range_changed(PlaybackViewManager* r,
                              const Range<double>& range) {
    repaint();
}

void Ruler::visible_range_changed(PlaybackViewManager* r,
                                  const Range<double>& range) {
    repaint();
}

void Ruler::current_time_changed(PlaybackViewManager* r, double time) {
    //    repaint();
}