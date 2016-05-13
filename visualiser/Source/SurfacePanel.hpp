#pragma once

#include "ModelWrapper.hpp"
#include "ValueWrapperSlider.hpp"

class VolumeComponent : public Component {
public:
    class VolumeSlider : public ValueWrapperSlider<float> {
    public:
        VolumeSlider(model::ValueWrapper<float>& value);

        float slider_to_value(float t) override;
        float value_to_slider(float t) override;
    };

    VolumeComponent(model::VolumeTypeWrapper& value);

    void resized() override;

private:
    model::VolumeTypeWrapper& value;

    VolumeSlider s0;
    VolumeSlider s1;
    VolumeSlider s2;
    VolumeSlider s3;
    VolumeSlider s4;
    VolumeSlider s5;
    VolumeSlider s6;
    VolumeSlider s7;

    std::array<std::reference_wrapper<VolumeSlider>, 8> get_slider_array();
};

//----------------------------------------------------------------------------//

class VolumeProperty : public PropertyComponent {
public:
    VolumeProperty(const String& name, model::VolumeTypeWrapper& value);
    void refresh() override;

private:
    VolumeComponent editor;
};

//----------------------------------------------------------------------------//

class FrequencyLabelComponent : public Component {
public:
    FrequencyLabelComponent();
    void resized() override;

private:
    Label l0;
    Label l1;
    Label l2;
    Label l3;
    Label l4;
    Label l5;
    Label l6;
    Label l7;

    std::array<std::reference_wrapper<Label>, 8> get_label_array();
};

//----------------------------------------------------------------------------//

class FrequencyLabelProperty : public PropertyComponent {
public:
    FrequencyLabelProperty(const String& name);
    void refresh() override;

private:
    FrequencyLabelComponent label;
};

//----------------------------------------------------------------------------//

class SurfaceComponent : public Component {
public:
    SurfaceComponent(model::SurfaceWrapper& value);
    void resized() override;

private:
    PropertyPanel property_panel;
};