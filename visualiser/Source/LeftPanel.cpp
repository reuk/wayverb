#include "LeftPanel.hpp"

#include "SurfacePanel.hpp"
#include "Vec3Editor.hpp"

class MaterialConfigureButton : public ButtonPropertyComponent {
public:
    MaterialConfigureButton(SurfaceModel::MaterialWrapper& value,
                            SurfaceModel& preset_model)
            : ButtonPropertyComponent(value.name.get_value(), true)
            , value(value)
            , preset_model(preset_model) {
    }

    String getButtonText() const override {
        return "...";
    }

    void buttonClicked() override {
        auto panel = new SurfaceComponentWithTitle(value, preset_model);
        CallOutBox::launchAsynchronously(panel, getScreenBounds(), nullptr);
    }

private:
    SurfaceModel::MaterialWrapper& value;
    SurfaceModel& preset_model;
};

//  ew non-managed pointers
//  yuck
Array<PropertyComponent*> make_material_buttons(const SurfaceModel& model,
                                                SurfaceModel& preset,
                                                LookAndFeel& laf) {
    Array<PropertyComponent*> ret;
    for (const auto& i : model.get_material_wrappers()) {
        auto to_add = new MaterialConfigureButton(i, preset);
        to_add->setLookAndFeel(&laf);
        ret.add(to_add);
    }
    return ret;
}

LeftPanel::LeftPanel(model::Combined& combined_model,
                     SurfaceModel& surface_model,
                     RenderStateManager& render_state_manager)
        : combined_model(combined_model)
        , surface_model(surface_model)
        , render_state_manager(render_state_manager)
        , bottom_panel(render_state_manager) {
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