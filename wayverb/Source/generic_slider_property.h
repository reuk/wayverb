#pragma once

#include "generic_property_component.h"

template <typename Model>
class generic_slider_property
        : public generic_property_component<Model, double, Slider> {
public:
    generic_slider_property(

            Model& model,
            const String& name,
            double min,
            double max,
            double inc = 0)
            : generic_property_component<Model, double, Slider>{
                      model,
                      name,
                      25,
                      Slider::SliderStyle::IncDecButtons,
                      Slider::TextEntryBoxPosition::TextBoxLeft} {
        this->content.setIncDecButtonsMode(
                Slider::IncDecButtonMode::incDecButtonsDraggable_AutoDirection);
        this->content.setRange(min, max, inc);
    }

    void sliderValueChanged(Slider* s) override {
        this->controller_updated(s->getValue());
    }

protected:
    ~generic_slider_property() noexcept = default;

private:
    void set_view(const double& value) override {
        this->content.setValue(value, dontSendNotification);
    }
};
