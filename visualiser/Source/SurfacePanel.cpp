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
    set_range(0.01, 1, 0);

    changeListenerCallback(&value);
}

// float VolumeComponent::VolumeSlider::slider_to_value(float t) {
//    return Decibels::decibelsToGain(t);
//}
// float VolumeComponent::VolumeSlider::value_to_slider(float t) {
//    return Decibels::gainToDecibels(t);
//}

//----------------------------------------------------------------------------//

VolumeComponent::VolumeComponent(model::ValueWrapper<VolumeType>& value)
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
                               model::ValueWrapper<VolumeType>& value)
        : PropertyComponent(name, 120)
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

SurfaceComponent::SurfaceComponent(
    model::ValueWrapper<Surface>& value,
    model::ValueWrapper<std::vector<SceneData::Material>>& preset_model) {
    property_panel.addProperties(
        {new FrequencyLabelProperty("frequencies / KHz")});
    property_panel.addProperties(
        {new VolumeProperty("diffuse gain", value.diffuse),
         new VolumeProperty("specular gain", value.specular)});
    property_panel.addProperties({new PresetProperty(value, preset_model)});

    property_panel.setOpaque(false);

    addAndMakeVisible(property_panel);

    setSize(400, property_panel.getTotalContentHeight());
}

void SurfaceComponent::resized() {
    property_panel.setBounds(getLocalBounds());
}

//----------------------------------------------------------------------------//

SurfaceComponentWithTitle::SurfaceComponentWithTitle(
    model::ValueWrapper<SceneData::Material>& value,
    model::ValueWrapper<std::vector<SceneData::Material>>& preset_model)
        : title("", value.name.get_value() + " settings")
        , surface_component(value.surface, preset_model) {
    title.setJustificationType(Justification::centred);

    addAndMakeVisible(title);
    addAndMakeVisible(surface_component);

    setSize(surface_component.getWidth(),
            surface_component.getHeight() + title_height);
}

void SurfaceComponentWithTitle::resized() {
    title.setBounds(getLocalBounds().removeFromTop(title_height));
    surface_component.setBounds(getLocalBounds().withTrimmedTop(title_height));
}

//----------------------------------------------------------------------------//

PresetComponent::PresetComponent(
    model::ValueWrapper<Surface>& linked,
    model::ValueWrapper<std::vector<SceneData::Material>>& preset_model)
        : linked(linked)
        , preset_model(preset_model) {
    combo_box.setEditableText(false);

    addChildComponent(combo_box);
    addChildComponent(text_editor);

    addAndMakeVisible(save_button);
    addAndMakeVisible(delete_button);

    changeListenerCallback(&preset_model);
}

PresetComponent::~PresetComponent() noexcept {
    PopupMenu::dismissAllActiveMenus();
}

void PresetComponent::resized() {
    auto bottom = getLocalBounds();
    auto combo_bounds =
        bottom.withHeight(bottom.getHeight() / 2).withTrimmedBottom(1);
    combo_box.setBounds(combo_bounds.reduced(1, 0));
    text_editor.setBounds(combo_bounds.reduced(1, 0));

    auto button_bounds = combo_bounds.withY(combo_bounds.getBottom() + 2);
    save_button.setBounds(
        button_bounds.withWidth(button_bounds.getWidth() / 2).reduced(1, 0));
    delete_button.setBounds(
        button_bounds.withTrimmedLeft(button_bounds.getWidth() / 2)
            .reduced(1, 0));
}

void PresetComponent::comboBoxChanged(ComboBox* cb) {
    if (cb == &combo_box) {
        auto selected = combo_box.getSelectedItemIndex();
        if (0 <= selected) {
            linked.set_value(preset_model[selected].surface);
            combo_box.setSelectedItemIndex(selected, dontSendNotification);
        }
    }
}

void PresetComponent::textEditorReturnKeyPressed(TextEditor& e) {
    if (e.getText().isNotEmpty()) {
        //  create new entry in model using current material settings
        preset_model.push_back(
            SceneData::Material{e.getText().toStdString(), linked});

        //  update combobox view
        combo_box.setSelectedItemIndex(preset_model.size() - 1,
                                       sendNotificationSync);
    } else {
        changeListenerCallback(&linked);
    }
}

void PresetComponent::buttonClicked(Button* b) {
    if (b == &save_button) {
        combo_box.setVisible(false);
        text_editor.setVisible(true);

        save_button.setVisible(false);
        delete_button.setVisible(false);

        text_editor.setText("new preset");
        text_editor.selectAll();

    } else if (b == &delete_button) {
        auto ind = combo_box.getSelectedItemIndex();
        if (0 <= ind) {
            preset_model.erase(ind);
        }
    }
}

void PresetComponent::changeListenerCallback(ChangeBroadcaster* cb) {
    text_editor.setVisible(false);
    combo_box.setVisible(true);

    save_button.setVisible(true);
    delete_button.setVisible(true);

    if (cb == &linked) {
        std::cout << "linked model changed" << std::endl;

        combo_box.setSelectedItemIndex(-1, dontSendNotification);
    } else if (cb == &preset_model) {
        combo_box.clear();

        auto counter = 1;
        for (const auto& i : preset_model) {
            combo_box.addItem(i->name.get_value(), counter++);
        }
    }
}

//----------------------------------------------------------------------------//

PresetProperty::PresetProperty(
    model::ValueWrapper<Surface>& linked,
    model::ValueWrapper<std::vector<SceneData::Material>>& preset_model)
        : PropertyComponent("presets", 52)
        , preset_component(linked, preset_model) {
    addAndMakeVisible(preset_component);
}
void PresetProperty::refresh() {
}