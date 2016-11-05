#include "GainSlider.hpp"

#include "utilities/range.h"

namespace gain_transform {

static const auto skew_factor =
        std::log(0.5) /
        std::log((-10.0 - db_minimum) / (db_maximum - db_minimum));

double db_gain_to_proportion(double u) {
    return std::pow(map(u,
                        util::make_range(db_minimum, db_maximum),
                        util::make_range(0.0, 1.0)),
                    skew_factor);
}

double proportion_to_db_gain(double u) {
    return map(0.0 < u ? std::exp(std::log(u) / skew_factor) : 0.0,
               util::make_range(0.0, 1.0),
               util::make_range(db_minimum, db_maximum));
}

}  // namespace gain_transform

GainSlider::GainSlider(Orientation orientation)
        : slider(orientation == Orientation::horizontal
                         ? juce::Slider::SliderStyle::LinearHorizontal
                         : juce::Slider::SliderStyle::LinearVertical,
                 juce::Slider::TextEntryBoxPosition::NoTextBox) {
    slider.setRange(0, 1);
    slider.setDoubleClickReturnValue(true, 1);
    slider.setValue(1, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(slider);
    slider.addListener(this);
}

GainSlider::~GainSlider() noexcept {
    slider.removeListener(this);
}

void GainSlider::resized() {
    slider.setBounds(getLocalBounds());
}

void GainSlider::sliderValueChanged(juce::Slider* s) {
    if (s == &slider) {
        listener_list.call(&Listener::gain_slider_value_changed, this);
    }
}

void GainSlider::sliderDragStarted(juce::Slider* s) {
    if (s == &slider) {
        listener_list.call(&Listener::gain_slider_drag_started, this);
    }
}

void GainSlider::sliderDragEnded(juce::Slider* s) {
    if (s == &slider) {
        listener_list.call(&Listener::gain_slider_drag_ended, this);
    }
}

double GainSlider::get_gain() const {
    const auto proportion = slider.getValue();
    const auto db = gain_transform::proportion_to_db_gain(proportion);
    const auto gain = juce::Decibels::decibelsToGain(db);
    return gain;
}

void GainSlider::set_gain(double g, juce::NotificationType n) {
    const auto db = juce::Decibels::gainToDecibels(g);
    const auto proportion = gain_transform::db_gain_to_proportion(db);
    slider.setValue(proportion, n);
}

void GainSlider::addListener(Listener* l) {
    listener_list.add(l);
}

void GainSlider::removeListener(Listener* l) {
    listener_list.remove(l);
}

//  slider range                0       0.5     1
//  db range                    -100    -10     0
//  gain range                  0       ~0.32   1
