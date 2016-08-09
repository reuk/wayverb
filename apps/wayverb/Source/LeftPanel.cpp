#include "LeftPanel.hpp"

#include "DirectionEditor.hpp"
#include "MicrophoneEditor.hpp"
#include "PropertyComponentLAF.hpp"
#include "SourcesEditor.hpp"
#include "SurfacePanel.hpp"
#include "TextDisplayProperty.hpp"
#include "Vec3Editor.hpp"

namespace {
class ConfigureButton : public ButtonPropertyComponent {
public:
    ConfigureButton(const String& name)
            : ButtonPropertyComponent(name, true) {
        setLookAndFeel(&pclaf);
    }

    String getButtonText() const override { return "..."; }

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
            model::ValueWrapper<copyable_scene_data::material>& value,
            model::ValueWrapper<aligned::vector<copyable_scene_data::material>>&
                    preset_model)
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
            shown_surface.set(
                    shown_surface.get() == this_surface ? -1 : this_surface);
        } else if (b == &more_button) {
            auto panel = new SurfaceComponentWithTitle(value, preset_model);
            CallOutBox::launchAsynchronously(
                    panel, more_button.getScreenBounds(), nullptr);
        }
    }

    void receive_broadcast(model::Broadcaster* b) override {
        if (b == &shown_surface) {
            show_button.setToggleState(shown_surface.get() == this_surface,
                                       dontSendNotification);
        }
    }

private:
    int this_surface;
    model::ValueWrapper<int>& shown_surface;
    model::BroadcastConnector surface_connector{&shown_surface, this};

    model::ValueWrapper<copyable_scene_data::material>& value;
    model::ValueWrapper<aligned::vector<copyable_scene_data::material>>&
            preset_model;

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
            model::ValueWrapper<copyable_scene_data::material>& value,
            model::ValueWrapper<aligned::vector<copyable_scene_data::material>>&
                    preset_model)
            : PropertyComponent(value.name.get())
            , contents(this_surface, shown_surface, value, preset_model) {
        set_help("surface material",
                 "Click 'show' to highlight everwhere this surface can be "
                 "found in the model. Click the '...' button to configure how "
                 "this surface should sound.");
        setLookAndFeel(&pclaf);
        addAndMakeVisible(contents);
    }

    void refresh() override {}

private:
    MaterialConfigureButtonComponent contents;
    PropertyComponentLAF pclaf{200};
};

Array<PropertyComponent*> make_material_buttons(
        model::ValueWrapper<int>& shown_surface,
        const model::ValueWrapper<
                aligned::vector<copyable_scene_data::material>>& model,
        model::ValueWrapper<aligned::vector<copyable_scene_data::material>>&
                preset) {
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

constexpr auto to_id(model::ReceiverSettings::Mode m) {
    switch (m) {
        case model::ReceiverSettings::Mode::hrtf: return 1;
        case model::ReceiverSettings::Mode::microphones: return 2;
    }
}

constexpr auto to_enum(int i) {
    switch (i) {
        case 1: return model::ReceiverSettings::Mode::hrtf;
        case 2: return model::ReceiverSettings::Mode::microphones;
    }
    return model::ReceiverSettings::Mode::hrtf;
}

//----------------------------------------------------------------------------//

class RayNumberPicker : public Component,
                        public ComboBox::Listener,
                        public model::BroadcastListener {
public:
    RayNumberPicker(model::ValueWrapper<size_t>& value)
            : value(value) {
        combo_box.addItem("few (1000)", 1000);
        combo_box.addItem("some (10 000)", 10000);
        combo_box.addItem("lots (100 000)", 100000);
        combo_box.addItem("insane (1 000 000)", 1000000);

        value_connector.trigger();
        addAndMakeVisible(combo_box);
    }

    virtual ~RayNumberPicker() noexcept { PopupMenu::dismissAllActiveMenus(); }

    void comboBoxChanged(ComboBox* cb) override {
        if (cb == &combo_box) {
            value.set(combo_box.getSelectedId());
        }
    }

    void receive_broadcast(model::Broadcaster* cb) override {
        if (cb == &value) {
            auto id = std::pow(
                    10,
                    std::floor(std::log10(static_cast<float>(value.get()))));
            combo_box.setSelectedId(id, dontSendNotification);
        }
    }

    void resized() override { combo_box.setBounds(getLocalBounds()); }

private:
    model::ValueWrapper<size_t>& value;
    model::BroadcastConnector value_connector{&value, this};

    ComboBox combo_box;
    model::Connector<ComboBox> combo_box_connector{&combo_box, this};
};

class RayNumberPickerProperty : public PropertyComponent,
                                public SettableHelpPanelClient {
public:
    RayNumberPickerProperty(model::ValueWrapper<size_t>& value)
            : PropertyComponent("rays")
            , picker(value) {
        set_help("ray number picker",
                 "Choose the number of rays that should be used for the "
                 "simulation. More rays will sound better but take longer to "
                 "simulate.");
        addAndMakeVisible(picker);
    }

    void refresh() override {}

private:
    RayNumberPicker picker;
};

//----------------------------------------------------------------------------//

class SourcesConfigureButton : public ConfigureButton {
public:
    SourcesConfigureButton(
            model::ValueWrapper<aligned::vector<glm::vec3>>& model,
            const box<3>& aabb)
            : ConfigureButton("sources")
            , model(model)
            , aabb(aabb) {}

    void buttonClicked() override {
        auto cmp = std::make_unique<SourcesEditorPanel>(model, aabb);
        cmp->setSize(500, 200);
        CallOutBox::launchAsynchronously(
                cmp.release(), getScreenBounds(), nullptr);
    }

private:
    model::ValueWrapper<aligned::vector<glm::vec3>>& model;
    box<3> aabb;
};

class ReceiversConfigureButton : public ConfigureButton {
public:
    ReceiversConfigureButton(
            model::ValueWrapper<aligned::vector<model::ReceiverSettings>>&
                    model,
            const box<3>& aabb)
            : ConfigureButton("receivers")
            , model(model)
            , aabb(aabb) {}

    void buttonClicked() override {
        auto cmp = std::make_unique<ReceiversEditorPanel>(model, aabb);
        cmp->setSize(800, 400);
        CallOutBox::launchAsynchronously(
                cmp.release(), getScreenBounds(), nullptr);
    }

private:
    model::ValueWrapper<aligned::vector<model::ReceiverSettings>>& model;
    box<3> aabb;
};

}  // namespace

//----------------------------------------------------------------------------//

LeftPanel::LeftPanel(model::ValueWrapper<model::FullModel>& model,
                     const box<3>& aabb)
        : model(model)
        , bottom_panel(model.render_state) {
    set_help("configuration panel",
             "Use the options in this panel to adjust the various settings of "
             "the simulation.");

    for (auto i : {&filter_frequency_connector, &oversample_ratio_connector}) {
        i->trigger();
    }

    {
        auto sources_property = std::make_unique<SourcesConfigureButton>(
                model.persistent.app.source, aabb);
        auto receivers_property = std::make_unique<ReceiversConfigureButton>(
                model.persistent.app.receiver_settings, aabb);

        property_panel.addSection(
                "general",
                {static_cast<PropertyComponent*>(sources_property.release()),
                 static_cast<PropertyComponent*>(
                         receivers_property.release())});
    }

    {
        Array<PropertyComponent*> materials;
        materials.addArray(make_material_buttons(model.shown_surface,
                                                 model.persistent.materials,
                                                 model.presets));
        property_panel.addSection("materials", materials);
    }

    {
        Array<PropertyComponent*> waveguide;
        waveguide.addArray({new NumberProperty<float>(
                                    "cutoff",
                                    model.persistent.app.filter_frequency,
                                    20,
                                    20000),
                            new NumberProperty<float>(
                                    "oversample",
                                    model.persistent.app.oversample_ratio,
                                    1,
                                    4)});
        waveguide.add(new TextDisplayProperty<int>(
                "waveguide sr / Hz", 20, waveguide_sampling_rate_wrapper));
        property_panel.addSection("waveguide", waveguide);
    }

    {
        property_panel.addSection(
                "raytracer",
                {new RayNumberPickerProperty(model.persistent.app.rays)});
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
        property_panel.setEnabled(!model.render_state.is_rendering.get());
    } else if (cb == &model.persistent.app.filter_frequency ||
               cb == &model.persistent.app.oversample_ratio) {
        waveguide_sampling_rate_wrapper.set(
                model.persistent.app.get().get_waveguide_sample_rate());
    }
}
