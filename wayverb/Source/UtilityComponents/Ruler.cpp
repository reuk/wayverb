#include "Ruler.hpp"
#include "Lerp.hpp"

#include <iomanip>
#include <sstream>

Ruler::Ruler(TransportViewManager &m)
        : transport_view_manager(m) {
    transport_view_manager.addListener(this);
}
Ruler::~Ruler() noexcept {
    transport_view_manager.removeListener(this);
}

void Ruler::paint(juce::Graphics &g) {
    g.setFillType(juce::FillType(
            juce::ColourGradient(juce::Colours::darkgrey,
                                 0,
                                 0,
                                 juce::Colours::darkgrey.darker(),
                                 0,
                                 getHeight(),
                                 false)));
    g.fillAll();
    g.setFillType(juce::FillType(juce::ColourGradient(juce::Colours::white,
                                                      0,
                                                      0,
                                                      juce::Colours::lightgrey,
                                                      0,
                                                      getHeight(),
                                                      false)));

    auto minDivision = 30;
    auto prec = std::ceil(
            std::log10((transport_view_manager.get_visible_range().getLength() *
                        minDivision) /
                       getWidth()));
    auto s = std::pow(10, prec);

    auto begin =
            std::floor(transport_view_manager.get_visible_range().getStart() /
                       s) *
            s;

    for (auto i = begin;
         i < transport_view_manager.get_visible_range().getEnd();
         i += s) {
        auto pos = lerp(i,
                        transport_view_manager.get_visible_range(),
                        juce::Range<double>(0, getWidth()));
        g.drawVerticalLine(pos, 2, getHeight() - 2);
        std::stringstream ss;
        ss << std::fixed << std::setprecision(std::max(0.0, -prec)) << i;
        g.setFont(10);
        g.drawText(
                ss.str(),
                pos + 2,
                0,
                s * getWidth() /
                        transport_view_manager.get_visible_range().getLength(),
                getHeight(),
                juce::Justification::left);
    }
}

double Ruler::x_to_time(double x) const {
    return lerp(x,
                juce::Range<double>(0, getWidth()),
                transport_view_manager.get_visible_range());
}

double Ruler::time_to_x(double time) const {
    return lerp(time,
                transport_view_manager.get_visible_range(),
                juce::Range<double>(0, getWidth()));
}

struct Ruler::RulerState {
    RulerState(double mouse_down_time, juce::Range<double> visible_range)
            : mouse_down_time(mouse_down_time)
            , visible_range(visible_range) {
    }
    double mouse_down_time;
    juce::Range<double> visible_range;
};

void Ruler::mouseEnter(const juce::MouseEvent &event) {
    setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
}

void Ruler::mouseDown(const juce::MouseEvent &e) {
    ruler_state = std::make_unique<RulerState>(
            x_to_time(e.getMouseDownX()),
            transport_view_manager.get_visible_range());
    e.source.enableUnboundedMouseMovement(true, false);
}

void Ruler::mouseUp(const juce::MouseEvent &event) {
    ruler_state = nullptr;
}

void Ruler::mouseExit(const juce::MouseEvent &event) {
    setMouseCursor(juce::MouseCursor::NormalCursor);
}

void Ruler::mouseDrag(const juce::MouseEvent &e) {
    assert(ruler_state);

    auto dy = -e.getDistanceFromDragStartY();

    auto doubleDist = 100.0;
    auto scale = pow(2.0, dy / doubleDist);

    auto w = ruler_state->visible_range.getLength() * scale;
    if (w >= transport_view_manager.get_max_length()) {
        transport_view_manager.reset_view();
    } else {
        //  the length of the time range to display
        auto clamped = std::max(0.001, w);

        //  move range so that ruler_state->mouse_down_time is at x
        auto x = e.getPosition().x;

        auto left =
                (0 - x) * clamped / getWidth() + ruler_state->mouse_down_time;
        auto right = (getWidth() - x) * clamped / getWidth() +
                     ruler_state->mouse_down_time;

        transport_view_manager.set_visible_range(
                juce::Range<double>(left, right), true);
    }
}

void Ruler::mouseDoubleClick(const juce::MouseEvent &event) {
    transport_view_manager.reset_view();
}

void Ruler::visible_range_changed(TransportViewManager *r,
                                  const juce::Range<double> &range) {
    repaint();
}
