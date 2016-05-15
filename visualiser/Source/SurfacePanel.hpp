#pragma once

#include "ModelWrapper.hpp"
#include "SurfaceModel.hpp"
#include "ValueWrapperSlider.hpp"

class VolumeComponent : public Component {
public:
    class VolumeSlider : public ValueWrapperSlider<float> {
    public:
        VolumeSlider(model::ValueWrapper<float>& value);

        //        float slider_to_value(float t) override;
        //        float value_to_slider(float t) override;
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

class PresetComponent : public Component,
                        public ComboBox::Listener,
                        public TextEditor::Listener,
                        public TextButton::Listener,
                        public ChangeListener {
public:
    PresetComponent(model::SurfaceWrapper& linked, SurfaceModel& preset_model);
    virtual ~PresetComponent() noexcept;
    void resized() override;

    void comboBoxChanged(ComboBox* cb) override;

    void buttonClicked(Button* b) override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

    void textEditorReturnKeyPressed(TextEditor& e) override;

private:
    model::SurfaceWrapper& linked;
    model::ChangeConnector linked_connector{&linked, this};

    SurfaceModel& preset_model;
    model::ChangeConnector preset_connector{&preset_model, this};

    ComboBox combo_box;
    TextEditor text_editor;
    TextButton save_button{"save"};
    TextButton delete_button{"delete"};
};

//----------------------------------------------------------------------------//

class PresetProperty : public PropertyComponent {
public:
    PresetProperty(model::SurfaceWrapper& linked, SurfaceModel& preset_model);
    void refresh() override;

private:
    PresetComponent preset_component;
};

//----------------------------------------------------------------------------//

class SurfaceComponent : public Component {
public:
    SurfaceComponent(model::SurfaceWrapper& value, SurfaceModel& preset_model);
    void resized() override;

private:
    PropertyPanel property_panel;
};

//----------------------------------------------------------------------------//

class SurfaceComponentWithTitle : public Component {
public:
    SurfaceComponentWithTitle(SurfaceModel::MaterialWrapper& value,
                              SurfaceModel& preset_model);
    void resized() override;

    static constexpr auto title_height = 25;

private:
    Label title;
    SurfaceComponent surface_component;
};