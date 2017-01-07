#include "slider_property.h"

namespace left_bar {

slider_property::slider_property(const String& name,
                                 double min,
                                 double max,
                                 double inc,
                                 const String& suffix)
        : PropertyComponent{name, 25}
        , slider_{Slider::SliderStyle::IncDecButtons,
                  Slider::TextEntryBoxPosition::TextBoxLeft} {
    slider_.setIncDecButtonsMode(
            Slider::IncDecButtonMode::incDecButtonsDraggable_AutoDirection);
    slider_.setTextBoxStyle(
            Slider::TextEntryBoxPosition::TextBoxLeft, false, 80, 21);
    slider_.setRange(min, max, inc);
    slider_.setTextValueSuffix(suffix);
    addAndMakeVisible(slider_);
}

double slider_property::get() const { return slider_.getValue(); }
void slider_property::set(double x) {
    slider_.setValue(x, dontSendNotification);
}

void slider_property::refresh() {}

slider_property::on_change::connection slider_property::connect_on_change(
        on_change::callback_type callback) {
    return on_change_.connect(std::move(callback));
}

void slider_property::sliderValueChanged(Slider*) { on_change_(*this, get()); }

}  // namespace left_bar {
