#include "SurfacePanel.hpp"

#include <functional>
#include <iomanip>
#include <sstream>

/*
VolumeComponent::VolumeSlider::VolumeSlider(model::ValueWrapper<float>& value)
        : ValueWrapperSlider<float>(value) {
    set_slider_style(Slider::SliderStyle::LinearVertical);
    set_text_box_style(Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    set_popup_display_enabled(true, nullptr);
    set_range(0.01, 0.99, 0);
}
*/

// float VolumeComponent::VolumeSlider::slider_to_value(float t) {
//    return Decibels::decibelsToGain(t);
//}
// float VolumeComponent::VolumeSlider::value_to_slider(float t) {
//    return Decibels::gainToDecibels(t);
//}

//----------------------------------------------------------------------------//

/*
VolumeComponent::VolumeComponent(model::ValueWrapper<volume_type>& value)
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
        sliders[i]->setBounds(total_width(i), 0, width, getHeight());
    }
}

std::array<VolumeComponent::VolumeSlider*, 8>
VolumeComponent::get_slider_array() {
    return std::array<VolumeSlider*, 8>{&s0, &s1, &s2, &s3, &s4, &s5, &s6, &s7};
}
*/

//----------------------------------------------------------------------------//

/*
VolumeProperty::VolumeProperty(const String& name,
                               model::ValueWrapper<volume_type>& value)
        : PropertyComponent(name, 120)
        , editor(value) {
    addAndMakeVisible(editor);
}

void VolumeProperty::refresh() {}
*/

//----------------------------------------------------------------------------//

/*
FrequencyLabelComponent::FrequencyLabelComponent() {
    std::array<std::string, 8> centres;

    std::transform(hrtf_data::edges.begin(),
                   hrtf_data::edges.end() - 1,
                   hrtf_data::edges.begin() + 1,
                   centres.begin(),
                   [](auto i, auto j) {
                       std::stringstream ss;
                       ss << std::setprecision(2) << (i + j) * 0.5 * 0.001;
                       return ss.str();
                   });

    auto labels = get_label_array();

    for (auto i = 0u; i != labels.size(); ++i) {
        labels[i]->setText(centres[i], NotificationType::dontSendNotification);
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
        labels[i]->setBounds(total_width(i), 0, width, getHeight());
    }
}

std::array<Label*, 8> FrequencyLabelComponent::get_label_array() {
    return std::array<Label*, 8>{&l0, &l1, &l2, &l3, &l4, &l5, &l6, &l7};
}
*/

//----------------------------------------------------------------------------//

/*
FrequencyLabelProperty::FrequencyLabelProperty(const String& name)
        : PropertyComponent(name) {
    addAndMakeVisible(label);
}

void FrequencyLabelProperty::refresh() {}

//----------------------------------------------------------------------------//

SurfaceComponent::SurfaceComponent(
        model::ValueWrapper<surface>& value,
        model::ValueWrapper<aligned::vector<scene_data::material>>&
                preset_model) {
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

void SurfaceComponent::resized() { property_panel.setBounds(getLocalBounds()); }

//----------------------------------------------------------------------------//

SurfaceComponentWithTitle::SurfaceComponentWithTitle(
        model::ValueWrapper<scene_data::material>& value,
        model::ValueWrapper<aligned::vector<scene_data::material>>&
                preset_model)
        : title("", value.name.get() + " settings")
        , surface_component(value.surface, preset_model) {
    set_help("surface configurator",
             "Move the sliders to adjust the per-band gain of the material for "
             "this surface, or choose an existing material from the combobox.");

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
        model::ValueWrapper<surface>& linked,
        model::ValueWrapper<aligned::vector<scene_data::material>>&
                preset_model)
        : linked(linked)
        , preset_model(preset_model) {
    set_help("preset selector",
             "The combobox contains a list of preset materials. To apply one, "
             "simply select it from the list. To save a new preset, click "
             "'save' while the sliders are in the desired position, then enter "
             "a name for the new preset and hit return.");
    combo_box.setEditableText(false);
    delete_button.setEnabled(false);

    addChildComponent(combo_box);
    addChildComponent(text_editor);

    addAndMakeVisible(save_button);
    addAndMakeVisible(delete_button);

    preset_connector.trigger();
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
    save_button.setBounds(button_bounds.withWidth(button_bounds.getWidth() / 2)
                                  .reduced(1, 0));
    delete_button.setBounds(
            button_bounds.withTrimmedLeft(button_bounds.getWidth() / 2)
                    .reduced(1, 0));
}

void PresetComponent::comboBoxChanged(ComboBox* cb) {
    if (cb == &combo_box) {
        auto selected = combo_box.getSelectedItemIndex();
        if (0 <= selected) {
            assert(preset_model[selected].name.get() ==
                   combo_box.getItemText(selected).toStdString());
            linked.set(preset_model[selected].surface.get());
            combo_box.setSelectedItemIndex(selected, dontSendNotification);
            delete_button.setEnabled(true);
        }
    }
}

void PresetComponent::textEditorReturnKeyPressed(TextEditor& e) {
    if (e.getText().isNotEmpty()) {
        //  create new entry in model using current material settings
        preset_model.push_back(
                scene_data::material{e.getText().toStdString(), linked.get()});

        //  update combobox view
        combo_box.setSelectedItemIndex(preset_model.size() - 1,
                                       sendNotificationSync);
    } else {
        linked_connector.trigger();
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

void PresetComponent::receive_broadcast(model::Broadcaster* cb) {
    text_editor.setVisible(false);
    combo_box.setVisible(true);

    save_button.setVisible(true);
    delete_button.setVisible(true);

    if (cb == &linked) {
        combo_box.setSelectedItemIndex(-1, dontSendNotification);
        delete_button.setEnabled(false);
    } else if (cb == &preset_model) {
        combo_box.clear();

        auto counter = 1;
        for (const auto& i : preset_model) {
            combo_box.addItem(i->name.get(), counter++);
        }
    }
}
*/

//----------------------------------------------------------------------------//

/*
PresetProperty::PresetProperty(
        model::ValueWrapper<surface>& linked,
        model::ValueWrapper<aligned::vector<scene_data::material>>&
                preset_model)
        : PropertyComponent("presets", 52)
        , preset_component(linked, preset_model) {
    addAndMakeVisible(preset_component);
}
void PresetProperty::refresh() {}
*/
