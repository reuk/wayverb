#include "SurfacePanel.hpp"

#include "common/hrtf.h"

#include <functional>
#include <iomanip>
#include <sstream>

VolumeComponent::VolumeSlider::VolumeSlider(model::ValueWrapper<float>& value)
        : ValueWrapperSlider<float>(value) {
    set_slider_style(Slider::SliderStyle::LinearVertical);
    set_text_box_style(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    set_popup_display_enabled(true, nullptr);
    set_range(-100, 0, 1);

    changeListenerCallback(&value);
}

float VolumeComponent::VolumeSlider::slider_to_value(float t) {
    return Decibels::decibelsToGain(t);
}
float VolumeComponent::VolumeSlider::value_to_slider(float t) {
    return Decibels::gainToDecibels(t);
}

//----------------------------------------------------------------------------//

VolumeComponent::VolumeComponent(model::VolumeTypeWrapper& value)
        : value(value)
        , s0(value.s0)
        , s1(value.s1)
        , s2(value.s2)
        , s3(value.s3)
        , s4(value.s4)
        , s5(value.s5)
        , s6(value.s6)
        , s7(value.s7) {
    for (auto& i : get_slider_array()) {
        addAndMakeVisible(i);
    }
}

void VolumeComponent::resized() {
    const auto bounds = getLocalBounds();

    const auto sliders = get_slider_array();

    auto total_width = [this, &sliders](auto i) {
        return i * getWidth() / sliders.size();
    };

    const auto width = total_width(1);

    for (auto i = 0u; i != sliders.size(); ++i) {
        sliders[i].get().setBounds(total_width(i), 0, width, getHeight());
    }
}

std::array<std::reference_wrapper<VolumeComponent::VolumeSlider>, 8>
VolumeComponent::get_slider_array() {
    return std::array<std::reference_wrapper<VolumeSlider>, 8>{
        std::ref(s0),
        std::ref(s1),
        std::ref(s2),
        std::ref(s3),
        std::ref(s4),
        std::ref(s5),
        std::ref(s6),
        std::ref(s7),
    };
}

//----------------------------------------------------------------------------//

VolumeProperty::VolumeProperty(const String& name,
                               model::VolumeTypeWrapper& value)
        : PropertyComponent(name, 100)
        , editor(value) {
    addAndMakeVisible(editor);
}

void VolumeProperty::refresh() {
}

//----------------------------------------------------------------------------//

FrequencyLabelComponent::FrequencyLabelComponent() {
    std::array<std::string, 8> centres;

    std::transform(HrtfData::EDGES.begin(),
                   HrtfData::EDGES.end() - 1,
                   HrtfData::EDGES.begin() + 1,
                   centres.begin(),
                   [](auto i, auto j) {
                       std::stringstream ss;
                       ss << std::setprecision(2) << (i + j) * 0.5 * 0.001;
                       return ss.str();
                   });

    auto labels = get_label_array();

    for (auto i = 0u; i != labels.size(); ++i) {
        labels[i].get().setText(centres[i],
                                NotificationType::dontSendNotification);
        addAndMakeVisible(labels[i]);
    }
}

void FrequencyLabelComponent::resized() {
    const auto bounds = getLocalBounds();

    const auto labels = get_label_array();

    auto total_width = [this, &labels](auto i) {
        return i * getWidth() / labels.size();
    };

    const auto width = total_width(1);

    for (auto i = 0u; i != labels.size(); ++i) {
        labels[i].get().setBounds(total_width(i), 0, width, getHeight());
    }
}

std::array<std::reference_wrapper<Label>, 8>
FrequencyLabelComponent::get_label_array() {
    return std::array<std::reference_wrapper<Label>, 8>{
        std::ref(l0),
        std::ref(l1),
        std::ref(l2),
        std::ref(l3),
        std::ref(l4),
        std::ref(l5),
        std::ref(l6),
        std::ref(l7),
    };
}

//----------------------------------------------------------------------------//

FrequencyLabelProperty::FrequencyLabelProperty(const String& name)
        : PropertyComponent(name) {
    addAndMakeVisible(label);
}

void FrequencyLabelProperty::refresh() {
}

//----------------------------------------------------------------------------//

SurfaceComponent::SurfaceComponent(model::SurfaceWrapper& value) {
    property_panel.addProperties(
        {new FrequencyLabelProperty("frequencies / KHz")});
    property_panel.addProperties(
        {new VolumeProperty("diffuse / dB", value.diffuse),
         new VolumeProperty("specular / dB", value.specular)});

    property_panel.setOpaque(false);

    addAndMakeVisible(property_panel);

    setSize(400, property_panel.getTotalContentHeight());
}

void SurfaceComponent::resized() {
    property_panel.setBounds(getLocalBounds());
}