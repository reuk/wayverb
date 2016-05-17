#include "LeftPanel.hpp"

#include "MicrophoneEditor.hpp"
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
    MaterialConfigureButton(model::ValueWrapper<SceneData::Material>& value,
                            model::Surfaces& preset_model)
            : ConfigureButton(value.name.get_value())
            , value(value)
            , preset_model(preset_model) {
    }

    void buttonClicked() override {
        auto panel = new SurfaceComponentWithTitle(value, preset_model);
        CallOutBox::launchAsynchronously(panel, getScreenBounds(), nullptr);
    }

private:
    model::ValueWrapper<SceneData::Material>& value;
    model::Surfaces& preset_model;
};

Array<PropertyComponent*> make_material_buttons(const model::Surfaces& model,
                                                model::Surfaces& preset,
                                                LookAndFeel& laf) {
    Array<PropertyComponent*> ret;
    for (const auto& i : model.get_wrapper()) {
        auto to_add = new MaterialConfigureButton(*i, preset);
        to_add->setLookAndFeel(&laf);
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

    void comboBoxChanged(ComboBox* cb) override {
        if (cb == &combo_box) {
            value.set_value(to_enum(combo_box.getSelectedId()));
        }
    }

    void changeListenerCallback(ChangeBroadcaster* cb) override {
        if (cb == &value) {
            combo_box.setSelectedId(to_id(value.get_value()),
                                    dontSendNotification);
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
        switch (receiver_config.mode.get_value()) {
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

LeftPanel::LeftPanel(model::Combined& combined_model,
                     model::Surfaces& surface_model,
                     RenderStateManager& render_state_manager)
        : combined_model(combined_model)
        , surface_model(surface_model)
        , render_state_manager(render_state_manager)
        , bottom_panel(render_state_manager)
        , preset_model(
              nullptr,
              std::vector<SceneData::Material>{
                  SceneData::Material{
                      "concrete_floor",
                      Surface{{0.99, 0.97, 0.95, 0.98, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "brickwork",
                      Surface{{0.99, 0.98, 0.98, 0.97, 0.97, 0.96, 0.96, 0.96},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "rough_lime_wash",
                      Surface{{0.98, 0.97, 0.96, 0.95, 0.96, 0.97, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "limestone",
                      Surface{{0.98, 0.98, 0.97, 0.96, 0.95, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "wooden_door",
                      Surface{{0.86, 0.9, 0.94, 0.92, 0.9, 0.9, 0.9, 0.9},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "rough_concrete",
                      Surface{{0.98, 0.97, 0.97, 0.97, 0.96, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "smooth_floor",
                      Surface{{0.98, 0.97, 0.97, 0.97, 0.97, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "lead_glazing",
                      Surface{{0.7, 0.8, 0.86, 0.9, 0.95, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "leaded_glazing",
                      Surface{{0.85, 0.7, 0.82, 0.9, 0.95, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "chairs__2",
                      Surface{{0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "takapaaty_flat__2",
                      Surface{{0.85, 0.9, 0.94, 0.96, 0.96, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "double_glazing",
                      Surface{{0.85, 0.95, 0.97, 0.97, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "audience_floor",
                      Surface{{0.91, 0.94, 0.95, 0.95, 0.95, 0.96, 0.96, 0.96},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "katto_flat__2",
                      Surface{{0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "verb_chamber",
                      Surface{{0.99, 0.99, 0.99, 0.98, 0.98, 0.96, 0.96, 0.96},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "plywood_panels",
                      Surface{{0.58, 0.79, 0.9, 0.92, 0.94, 0.94, 0.94, 0.94},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "smooth_concrete",
                      Surface{
                          {0.6, 0.8, 0.9, 0.95, 0.97, 0.97, 0.96, 0.95},
                          {0.99, 0.99, 0.98, 0.98, 0.98, 0.95, 0.95, 0.95}}},
                  SceneData::Material{
                      "glass_window",
                      Surface{{0.9, 0.95, 0.96, 0.97, 0.97, 0.97, 0.97, 0.97},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "ceramic_tiles",
                      Surface{{0.99, 0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "pile_carpet",
                      Surface{{0.97, 0.91, 0.75, 0.69, 0.6, 0.56, 0.56, 0.56},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "4cm_planks",
                      Surface{{0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "point_brickwork",
                      Surface{{0.92, 0.91, 0.88, 0.84, 0.78, 0.76, 0.76, 0.76},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "marble_floor",
                      Surface{{0.99, 0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "wooden_lining",
                      Surface{{0.73, 0.77, 0.78, 0.85, 0.9, 0.93, 0.94, 0.94},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "hull__2",
                      Surface{{0.99, 0.99, 0.99, 0.98, 0.98, 0.96, 0.96, 0.96},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "stucco_brick",
                      Surface{{0.97, 0.97, 0.97, 0.96, 0.95, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "vasen_flat__2",
                      Surface{{0.73, 0.77, 0.78, 0.85, 0.9, 0.93, 0.94, 0.94},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "double_glazing_2",
                      Surface{{0.9, 0.93, 0.95, 0.97, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "hard_wall",
                      Surface{{0.98, 0.98, 0.97, 0.97, 0.96, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "thin_carpet",
                      Surface{{0.98, 0.96, 0.92, 0.8, 0.75, 0.6, 0.6, 0.6},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "etupaaty_flat__2",
                      Surface{{0.92, 0.91, 0.88, 0.84, 0.78, 0.76, 0.76, 0.76},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "oikea_flat__2",
                      Surface{{0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "smooth_brickwork",
                      Surface{{0.99, 0.99, 0.98, 0.98, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "wood_40mm_studs",
                      Surface{{0.82, 0.88, 0.9, 0.91, 0.92, 0.93, 0.93, 0.93},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "cotton_carpet",
                      Surface{{0.93, 0.69, 0.51, 0.19, 0.34, 0.46, 0.52, 0.52},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "3mm_glass",
                      Surface{{0.92, 0.96, 0.97, 0.97, 0.98, 0.98, 0.98, 0.98},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "tufted_carpet",
                      Surface{{0.9, 0.6, 0.37, 0.3, 0.37, 0.12, 0.12, 0.12},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "plasterboard",
                      Surface{{0.85, 0.9, 0.94, 0.96, 0.96, 0.95, 0.95, 0.95},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
                  SceneData::Material{
                      "stage_floor",
                      Surface{{0.9, 0.93, 0.94, 0.94, 0.94, 0.94, 0.94, 0.94},
                              {0.95, 0.9, 0.85, 0.8, 0.75, 0.7, 0.65, 0.6}}},
              }) {
    receiver_config.get_wrapper().hrtf_model.facing.set_value(Vec3f{1, 0, 0});
    receiver_config.get_wrapper().hrtf_model.up.set_value(Vec3f{0, 1, 0});

    receiver_config.get_wrapper().microphone_model.microphones.push_back(
        config::Microphone{Vec3f{1, 0, 0}, 0.5});
    receiver_config.get_wrapper().microphone_model.microphones.push_back(
        config::Microphone{Vec3f{0, 0, 1}, 0.5});

    property_panel.addSection(
        "general",
        {new Vec3fProperty("source", combined_model.get_wrapper().source),
         new Vec3fProperty("mic", combined_model.get_wrapper().mic)});

    Array<PropertyComponent*> materials;
    materials.addArray(
        make_material_buttons(surface_model, preset_model, pclaf));
    property_panel.addSection("materials", materials);

    property_panel.addSection(
        "waveguide",
        {new NumberProperty<float>(
             "cutoff", combined_model.get_wrapper().filter_frequency),
         new NumberProperty<float>(
             "oversample", combined_model.get_wrapper().oversample_ratio)});

    property_panel.addSection(
        "raytracer",
        {new NumberProperty<int>("rays", combined_model.get_wrapper().rays),
         new NumberProperty<int>("reflections",
                                 combined_model.get_wrapper().impulses)});

    Array<PropertyComponent*> receivers;
    receivers.add(
        new ReceiverPickerProperty(receiver_config.get_wrapper().mode));
    receivers.add(new ReceiverConfigureButton(receiver_config.get_wrapper()));
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
}