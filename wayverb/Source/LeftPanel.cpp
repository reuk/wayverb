#include "LeftPanel.hpp"

#include "DirectionEditor.hpp"
#include "MicrophoneEditor.hpp"
#include "PropertyComponentLAF.hpp"
#include "SurfacePanel.hpp"
#include "TextDisplayProperty.hpp"
#include "Vec3Editor.hpp"

class ConfigureButton : public ButtonPropertyComponent {
public:
    ConfigureButton(const String& name)
            : ButtonPropertyComponent(name, true) {
        setLookAndFeel(&pclaf);
    }

    String getButtonText() const override {
        return "...";
    }

private:
    PropertyComponentLAF pclaf{200};
};

class MaterialConfigureButtonComponent : public Component,
                                         public Button::Listener,
                                         public model::BroadcastListener {
public:
    MaterialConfigureButtonComponent(
        int this_surface,
        model::ValueWrapper<int>& shown_surface,
        model::ValueWrapper<SceneData::Material>& value,
        model::ValueWrapper<std::vector<SceneData::Material>>& preset_model)
            : this_surface(this_surface)
            , shown_surface(shown_surface)
            , value(value)
            , preset_model(preset_model) {
        show_button.setClickingTogglesState(false);

        addAndMakeVisible(show_button);
        addAndMakeVisible(more_button);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        show_button.setBounds(bounds.removeFromLeft(50));
        bounds.removeFromLeft(2);
        more_button.setBounds(bounds);
    }

    void buttonClicked(Button* b) override {
        if (b == &show_button) {
            shown_surface.set(shown_surface == this_surface ? -1
                                                            : this_surface);
        } else if (b == &more_button) {
            auto panel = new SurfaceComponentWithTitle(value, preset_model);
            CallOutBox::launchAsynchronously(
                panel, more_button.getScreenBounds(), nullptr);
        }
    }

    void receive_broadcast(model::Broadcaster* b) override {
        if (b == &shown_surface) {
            show_button.setToggleState(shown_surface == this_surface,
                                       dontSendNotification);
        }
    }

private:
    int this_surface;
    model::ValueWrapper<int>& shown_surface;
    model::BroadcastConnector surface_connector{&shown_surface, this};

    model::ValueWrapper<SceneData::Material>& value;
    model::ValueWrapper<std::vector<SceneData::Material>>& preset_model;

    TextButton show_button{"show"};
    model::Connector<TextButton> show_connector{&show_button, this};

    TextButton more_button{"..."};
    model::Connector<TextButton> more_connector{&more_button, this};
};

class MaterialConfigureButton : public PropertyComponent,
                                public SettableHelpPanelClient {
public:
    MaterialConfigureButton(
        int this_surface,
        model::ValueWrapper<int>& shown_surface,
        model::ValueWrapper<SceneData::Material>& value,
        model::ValueWrapper<std::vector<SceneData::Material>>& preset_model)
            : PropertyComponent(value.name.get())
            , contents(this_surface, shown_surface, value, preset_model) {
        set_help("surface material",
                 "Click 'show' to highlight everwhere this surface can be "
                 "found in the model. Click the '...' button to configure how "
                 "this surface should sound.");
        setLookAndFeel(&pclaf);
        addAndMakeVisible(contents);
    }

    void refresh() override {
    }

private:
    MaterialConfigureButtonComponent contents;
    PropertyComponentLAF pclaf{200};
};

Array<PropertyComponent*> make_material_buttons(
    model::ValueWrapper<int>& shown_surface,
    const model::ValueWrapper<std::vector<SceneData::Material>>& model,
    model::ValueWrapper<std::vector<SceneData::Material>>& preset) {
    Array<PropertyComponent*> ret;
    auto count = 0;
    for (const auto& i : model) {
        auto to_add =
            new MaterialConfigureButton(count++, shown_surface, *i, preset);
        ret.add(to_add);
    }
    return ret;
}

//----------------------------------------------------------------------------//

static constexpr auto to_id(model::ReceiverSettings::Mode m) {
    switch (m) {
        case model::ReceiverSettings::Mode::hrtf:
            return 1;
        case model::ReceiverSettings::Mode::microphones:
            return 2;
    }
}

static constexpr auto to_enum(int i) {
    switch (i) {
        case 1:
            return model::ReceiverSettings::Mode::hrtf;
        case 2:
            return model::ReceiverSettings::Mode::microphones;
    }
    return model::ReceiverSettings::Mode::hrtf;
}

class ReceiverPicker : public Component,
                       public ComboBox::Listener,
                       public model::BroadcastListener {
public:
    ReceiverPicker(model::ValueWrapper<model::ReceiverSettings::Mode>& value)
            : value(value) {
        combo_box.addItem("microphones",
                          to_id(model::ReceiverSettings::Mode::microphones));
        combo_box.addItem("HRTF", to_id(model::ReceiverSettings::Mode::hrtf));

        value_connector.trigger();
        addAndMakeVisible(combo_box);
    }

    virtual ~ReceiverPicker() noexcept {
        PopupMenu::dismissAllActiveMenus();
    }

    void comboBoxChanged(ComboBox* cb) override {
        if (cb == &combo_box) {
            value.set(to_enum(combo_box.getSelectedId()));
        }
    }

    void receive_broadcast(model::Broadcaster* cb) override {
        if (cb == &value) {
            combo_box.setSelectedId(to_id(value), dontSendNotification);
        }
    }

    void resized() override {
        combo_box.setBounds(getLocalBounds());
    }

private:
    model::ValueWrapper<model::ReceiverSettings::Mode>& value;
    model::BroadcastConnector value_connector{&value, this};

    ComboBox combo_box;
    model::Connector<ComboBox> combo_box_connector{&combo_box, this};
};

class ReceiverPickerProperty : public PropertyComponent,
                               public SettableHelpPanelClient {
public:
    ReceiverPickerProperty(
        model::ValueWrapper<model::ReceiverSettings::Mode>& value)
            : PropertyComponent("receiver type")
            , receiver_picker(value) {
        set_help("receiver type picker",
                 "Choose whether the simulation should use simulated "
                 "microphones, or simulated ears (hrtf modelling).");
        addAndMakeVisible(receiver_picker);
    }

    void refresh() override {
    }

private:
    ReceiverPicker receiver_picker;
};

//----------------------------------------------------------------------------//

class HrtfModelComponent : public Component, public SettableHelpPanelClient {
public:
    HrtfModelComponent(model::ValueWrapper<model::Pointer>& hrtf,
                       model::ValueWrapper<glm::vec3>& mic_position,
                       model::ValueWrapper<glm::vec3>& source_position) {
        set_help("hrtf configurator",
                 "There's only one option, which allows you to choose the "
                 "direction that the virtual head should be facing.");
        property_panel.addProperties({new DirectionProperty(hrtf)});

        addAndMakeVisible(property_panel);

        setSize(400, property_panel.getTotalContentHeight());
    }

    void resized() override {
        property_panel.setBounds(getLocalBounds());
    }

private:
    PropertyPanel property_panel;
};

class ReceiverConfigureButton : public ConfigureButton {
public:
    ReceiverConfigureButton(
        model::ValueWrapper<model::ReceiverSettings>& receiver_settings,
        model::ValueWrapper<glm::vec3>& mic_position,
        model::ValueWrapper<glm::vec3>& source_position)
            : ConfigureButton("configure")
            , receiver_settings(receiver_settings)
            , mic_position(mic_position)
            , source_position(source_position) {
    }

    void buttonClicked() override {
        Component* c = nullptr;
        switch (receiver_settings.mode) {
            case model::ReceiverSettings::Mode::hrtf:
                c = new HrtfModelComponent(
                    receiver_settings.hrtf, mic_position, source_position);
                break;
            case model::ReceiverSettings::Mode::microphones:
                c = new MicrophoneEditorPanel(receiver_settings.microphones);
                break;
        }
        CallOutBox::launchAsynchronously(c, getScreenBounds(), nullptr);
    }

private:
    model::ValueWrapper<model::ReceiverSettings>& receiver_settings;
    model::ValueWrapper<glm::vec3>& mic_position;
    model::ValueWrapper<glm::vec3>& source_position;
};

//----------------------------------------------------------------------------//

LeftPanel::LeftPanel(model::ValueWrapper<model::FullModel>& model,
                     const CuboidBoundary& aabb)
        : model(model)
        , bottom_panel(model.render_state) {
    set_help("configuration panel",
             "Use the options in this panel to adjust the various settings of "
             "the simulation.");

    for (auto i : {&filter_frequency_connector, &oversample_ratio_connector}) {
        i->trigger();
    }

    {
        auto source_property = new Vec3Property("source",
                                                model.persistent.app.source,
                                                aabb.get_c0(),
                                                aabb.get_c1());
        auto mic_property = new Vec3Property("receiver",
                                             model.persistent.app.receiver,
                                             aabb.get_c0(),
                                             aabb.get_c1());

        source_property->set_help(
            "source position",
            "Allows you to move the source position in each "
            "of the axial directions.");
        mic_property->set_help(
            "receiver position",
            "Allows you to move the receiver position in each "
            "of the axial directions.");

        Array<PropertyComponent*> general{source_property, mic_property};
        general.add(new ReceiverPickerProperty(
            model.persistent.app.receiver_settings.mode));
        general.add(
            new ReceiverConfigureButton(model.persistent.app.receiver_settings,
                                        model.persistent.app.receiver,
                                        model.persistent.app.source));
        property_panel.addSection("general", general);
    }

    {
        Array<PropertyComponent*> materials;
        materials.addArray(make_material_buttons(
            model.shown_surface, model.persistent.materials, model.presets));
        property_panel.addSection("materials", materials);
    }

    {
        Array<PropertyComponent*> waveguide;
        waveguide.addArray(
            {new NumberProperty<float>(
                 "cutoff", model.persistent.app.filter_frequency, 20, 20000),
             new NumberProperty<float>(
                 "oversample", model.persistent.app.oversample_ratio, 1, 4)});
        waveguide.add(new TextDisplayProperty<int>(
            "waveguide sr / Hz", 20, waveguide_sampling_rate_wrapper));
        property_panel.addSection("waveguide", waveguide);
    }

    {
        property_panel.addSection(
            "raytracer",
            {new NumberProperty<int>(
                 "rays", model.persistent.app.rays, 1000, 1000000),
             new NumberProperty<int>(
                 "reflections", model.persistent.app.impulses, 20, 200)});
    }

    property_panel.setOpaque(false);

    addAndMakeVisible(property_panel);
    addAndMakeVisible(bottom_panel);
}

void LeftPanel::resized() {
    auto panel_height = 30;
    property_panel.setBounds(getLocalBounds().withTrimmedBottom(panel_height));
    bottom_panel.setBounds(getLocalBounds().removeFromBottom(panel_height));
}

void LeftPanel::receive_broadcast(model::Broadcaster* cb) {
    if (cb == &model.render_state.is_rendering) {
        property_panel.setEnabled(!model.render_state.is_rendering);
    } else if (cb == &model.persistent.app.filter_frequency ||
               cb == &model.persistent.app.oversample_ratio) {
        waveguide_sampling_rate_wrapper.set(
            model.persistent.app.get().get_waveguide_sample_rate());
    }
}
