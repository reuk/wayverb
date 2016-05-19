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
    PropertyComponentLAF pclaf;
};

class MaterialConfigureButton : public ConfigureButton {
public:
    MaterialConfigureButton(
        model::ValueWrapper<SceneData::Material>& value,
        model::ValueWrapper<std::vector<SceneData::Material>>& preset_model)
            : ConfigureButton(value.name.get())
            , value(value)
            , preset_model(preset_model) {
    }

    void buttonClicked() override {
        auto panel = new SurfaceComponentWithTitle(value, preset_model);
        CallOutBox::launchAsynchronously(panel, getScreenBounds(), nullptr);
    }

private:
    model::ValueWrapper<SceneData::Material>& value;
    model::ValueWrapper<std::vector<SceneData::Material>>& preset_model;
};

Array<PropertyComponent*> make_material_buttons(
    const model::ValueWrapper<std::vector<SceneData::Material>>& model,
    model::ValueWrapper<std::vector<SceneData::Material>>& preset) {
    Array<PropertyComponent*> ret;
    for (const auto& i : model) {
        auto to_add = new MaterialConfigureButton(*i, preset);
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
                       public ChangeListener {
public:
    ReceiverPicker(model::ValueWrapper<config::AttenuationModel::Mode>& value)
            : value(value) {
        combo_box.addItem("microphones",
                          to_id(config::AttenuationModel::Mode::microphone));
        combo_box.addItem("HRTF", to_id(config::AttenuationModel::Mode::hrtf));

        changeListenerCallback(&value);
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

    void changeListenerCallback(ChangeBroadcaster* cb) override {
        if (cb == &value) {
            combo_box.setSelectedId(to_id(value), dontSendNotification);
        }
    }

    void resized() override {
        combo_box.setBounds(getLocalBounds());
    }

private:
    model::ValueWrapper<config::AttenuationModel::Mode>& value;
    model::ChangeConnector value_connector{&value, this};

    ComboBox combo_box;
    model::Connector<ComboBox> combo_box_connector{&combo_box, this};
};

class ReceiverPickerProperty : public PropertyComponent {
public:
    ReceiverPickerProperty(
        model::ValueWrapper<config::AttenuationModel::Mode>& value)
            : PropertyComponent("receiver type")
            , receiver_picker(value) {
        addAndMakeVisible(receiver_picker);
    }

    void refresh() override {
    }

private:
    ReceiverPicker receiver_picker;
};

//----------------------------------------------------------------------------//

class HrtfModelComponent : public Component {
public:
    HrtfModelComponent(model::ValueWrapper<config::HrtfModel>& hrtf_model) {
        property_panel.addProperties(
            {new Vec3fProperty("facing", hrtf_model.facing),
             new Vec3fProperty("up", hrtf_model.up)});

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
        model::ValueWrapper<model::FullReceiverConfig>& receiver_config)
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
    model::ValueWrapper<model::FullReceiverConfig>& receiver_config;
};

//----------------------------------------------------------------------------//

LeftPanel::LeftPanel(model::ValueWrapper<model::FullModel>& model)
        : model(model)
        , bottom_panel(model.render_state) {
    property_panel.addSection(
        "general",
        {new Vec3fProperty("source", model.combined.source),
         new Vec3fProperty("mic", model.combined.mic)});

    Array<PropertyComponent*> materials;
    materials.addArray(make_material_buttons(model.materials, model.presets));
    property_panel.addSection("materials", materials);

    property_panel.addSection(
        "waveguide",
        {new NumberProperty<float>("cutoff", model.combined.filter_frequency),
         new NumberProperty<float>("oversample",
                                   model.combined.oversample_ratio)});

    property_panel.addSection(
        "raytracer",
        {new NumberProperty<int>("rays", model.combined.rays),
         new NumberProperty<int>("reflections", model.combined.impulses)});

    Array<PropertyComponent*> receivers;
    receivers.add(new ReceiverPickerProperty(model.receiver.mode));
    receivers.add(new ReceiverConfigureButton(model.receiver));
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

void LeftPanel::changeListenerCallback(ChangeBroadcaster* cb) {
    if (cb == &model.render_state.is_rendering) {
        property_panel.setEnabled(!model.render_state.is_rendering);
    }
}
