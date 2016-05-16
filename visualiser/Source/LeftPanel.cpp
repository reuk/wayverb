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