#include "RightPanel.hpp"

#include "LoadFiles.hpp"

RightPanel::RightPanel(model::Combined& combined_model,
                       SceneData& scene_data,
                       RenderStateManager& render_state_manager)
        : combined_model(combined_model)
        , scene_data(scene_data)
        , render_state_manager(render_state_manager)
        , model_renderer_component(
              render_state_manager, scene_data, combined_model.get_value()) {
    addAndMakeVisible(model_renderer_component);
}

void RightPanel::resized() {
    /*
        auto panel_height = 30;
        model_renderer_component.setBounds(
            getLocalBounds().withTrimmedBottom(panel_height));
        bottom_panel.setBounds(getLocalBounds().removeFromBottom(panel_height));
    */
    model_renderer_component.setBounds(getLocalBounds());
}

void RightPanel::save_as_project() {
    FileChooser fc("save project", File::nonexistent, "*.way");
    if (fc.browseForFileToSave(true)) {
        //  TODO
    }
}
