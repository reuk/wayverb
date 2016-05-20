#pragma once

#include "FullModel.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

template <typename T>
class ValueWrapperSlider : public Component,
                           public Slider::Listener,
                           public model::BroadcastListener {
public:
    ValueWrapperSlider(model::ValueWrapper<T>& value)
            : value(value) {
        slider.addListener(this);
        addAndMakeVisible(slider);
    }

    virtual ~ValueWrapperSlider() noexcept {
        slider.removeListener(this);
    }

    virtual T slider_to_value(T t) {
        return t;
    }

    virtual T value_to_slider(T t) {
        return t;
    }

    void sliderValueChanged(Slider* s) override {
        if (s == &slider) {
            value.set(slider_to_value(slider.getValue()));
        }
    }

    void receive_broadcast(model::Broadcaster* cb) override {
        if (cb == &value) {
            slider.setValue(value_to_slider(value), dontSendNotification);
        }
    }

    void resized() override {
        slider.setBounds(getLocalBounds());
    }

    void set_slider_style(Slider::SliderStyle s) {
        slider.setSliderStyle(s);
    }

    void set_text_box_style(Slider::TextEntryBoxPosition p,
                            bool read_only,
                            int w,
                            int h) {
        slider.setTextBoxStyle(p, read_only, w, h);
    }

    void set_popup_display_enabled(bool enabled, Component* parent) {
        slider.setPopupDisplayEnabled(enabled, parent);
    }

    void set_range(double min, double max, double interval = 0) {
        slider.setRange(min, max, interval);
    }

private:
    model::ValueWrapper<T>& value;
    model::BroadcastConnector value_connector{&value, this};
    Slider slider;
};