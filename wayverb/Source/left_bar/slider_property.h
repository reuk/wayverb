#pragma once

#include "../UtilityComponents/connector.h"

#include "utilities/event.h"

#include "../JuceLibraryCode/JuceHeader.h"

namespace left_bar {

class slider_property final : public PropertyComponent,
                              public Slider::Listener {
public:
    slider_property(const String& name, double min, double max, double inc=0, const String& suffix="");

    double get() const;
    void set(double x);

    void refresh() override;

    using on_change = util::event<slider_property&, double>;
    on_change::connection connect_on_change(on_change::callback_type callback);

    void sliderValueChanged(Slider*) override;

private:
    Slider slider_;
    model::Connector<Slider> slider_connector_{&slider_, this};
    on_change on_change_;
};

}  // namespace left_bar {
