#include "LeftPanel.hpp"

#include "MicrophoneEditor.hpp"
#include "PropertyComponentLAF.hpp"
#include "SurfacePanel.hpp"
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

static constexpr auto to_id(config::AttenuationModel::Mode m) {
    switch (m) {
        case config::AttenuationModel::Mode::hrtf:
            return 1;
        case config::AttenuationModel::Mode::microphone:
            return 2;
    }
}

static constexpr auto to_enum(int i) {
    switch (i) {
        case 1:
            return config::AttenuationModel::Mode::hrtf;
        case 2:
            return config::AttenuationModel::Mode::microphone;
    }
    return config::AttenuationModel::Mode::hrtf;
}

class ReceiverPicker : public Component,
                       public ComboBox::Listener,
                       public model::BroadcastListener {
public:
    ReceiverPicker(model::ValueWrapper<config::AttenuationModel::Mode>& value)
            : value(value) {
        combo_box.addItem("microphones",
                          to_id(config::AttenuationModel::Mode::microphone));
        combo_box.addItem("HRTF", to_id(config::AttenuationModel::Mode::hrtf));

        receive_broadcast(&value);
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
    model::ValueWrapper<config::AttenuationModel::Mode>& value;
    model::BroadcastConnector value_connector{&value, this};

    ComboBox combo_box;
    model::Connector<ComboBox> combo_box_connector{&combo_box, this};
};

class ReceiverPickerProperty : public PropertyComponent,
                               public SettableHelpPanelClient {
public:
    ReceiverPickerProperty(
        model::ValueWrapper<config::AttenuationModel::Mode>& value)
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
    HrtfModelComponent(model::ValueWrapper<config::HrtfModel>& hrtf_model) {
        set_help("hrtf configurator",
                 "There's only one option, which allows you to choose the "
                 "direction that the virtual head should be facing.");
        property_panel.addProperties({new Vec3fProperty(
            "facing", hrtf_model.facing, Vec3f(-1), Vec3f(1))});

        addAndMakeVisible(property_panel);

        setSize(300, property_panel.getTotalContentHeight());
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
        model::ValueWrapper<config::ReceiverConfig>& receiver_config)
            : ConfigureButton("configure")
            , receiver_config(receiver_config) {
    }

    void buttonClicked() override {
        Component* c = nullptr;
        switch (receiver_config.mode) {
            case config::AttenuationModel::Mode::hrtf:
                c = new HrtfModelComponent(receiver_config.hrtf_model);
                break;
            case config::AttenuationModel::Mode::microphone:
                c = new MicrophoneEditorPanel(receiver_config.microphone_model);
                break;
        }
        CallOutBox::launchAsynchronously(c, getScreenBounds(), nullptr);
    }

private:
    model::ValueWrapper<config::ReceiverConfig>& receiver_config;
};

//----------------------------------------------------------------------------//

LeftPanel::LeftPanel(model::ValueWrapper<model::FullModel>& model,
                     const CuboidBoundary& aabb)
        : model(model)
        , bottom_panel(model.render_state) {
    set_help("configuration panel",
             "Use the options in this panel to adjust the various settings of "
             "the simulation.");
    property_panel.addSection(
        "general",
        {new Vec3fProperty("source",
                           model.persistent.combined.source,
                           aabb.get_c0(),
                           aabb.get_c1()),
         new Vec3fProperty("mic",
                           model.persistent.combined.mic,
                           aabb.get_c0(),
                           aabb.get_c1())});

    Array<PropertyComponent*> materials;
    materials.addArray(make_material_buttons(
        model.shown_surface, model.persistent.materials, model.presets));
    property_panel.addSection("materials", materials);

    property_panel.addSection(
        "waveguide",
        {new NumberProperty<float>(
             "cutoff", model.persistent.combined.filter_frequency, 20, 20000),
         new NumberProperty<float>(
             "oversample", model.persistent.combined.oversample_ratio, 1, 4)});

    property_panel.addSection(
        "raytracer",
        {new NumberProperty<int>(
             "rays", model.persistent.combined.rays, 1000, 1000000),
         new NumberProperty<int>(
             "reflections", model.persistent.combined.impulses, 20, 200)});

    Array<PropertyComponent*> receivers;
    receivers.add(new ReceiverPickerProperty(
        model.persistent.combined.receiver_config.mode));
    receivers.add(
        new ReceiverConfigureButton(model.persistent.combined.receiver_config));
    property_panel.addSection("receiver", receivers);

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
    }
}
