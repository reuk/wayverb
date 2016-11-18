#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace gain_transform {

static const auto db_minimum = -100.0;
static const auto db_maximum = 0.0;
double db_gain_to_proportion(double gain);
double proportion_to_db_gain(double prop);

}  // namespace gain_transform

class GainSlider : public juce::Component, public juce::Slider::Listener {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(const Listener&) = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() = default;

        virtual void gain_slider_value_changed(GainSlider*) = 0;
        virtual void gain_slider_drag_started(GainSlider*) {}
        virtual void gain_slider_drag_ended(GainSlider*) {}
    };

    enum class Orientation { horizontal, vertical };

    GainSlider(Orientation orientation);
    virtual ~GainSlider() noexcept;

    void resized() override;

    void sliderValueChanged(juce::Slider*) override;
    void sliderDragStarted(juce::Slider*) override;
    void sliderDragEnded(juce::Slider*) override;

    double get_gain() const;
    void set_gain(double g, juce::NotificationType notify);

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    juce::ListenerList<Listener> listener_list;
    juce::Slider slider;
};
