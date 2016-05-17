#include "RightPanel.hpp"

#include "LoadFiles.hpp"

RightPanel::RightPanel(
    SceneData& scene_data,
    model::ValueWrapper<config::Combined>& combined_model,
    model::ValueWrapper<model::RenderStateManager>& render_state_manager)
        : model_renderer_component(
              scene_data, combined_model, render_state_manager) {
    addAndMakeVisible(model_renderer_component);
}

void RightPanel::resized() {
    model_renderer_component.setBounds(getLocalBounds());
}

/*
void RightPanel::save_as_project() {
    FileChooser fc("save project", File::nonexistent, "*.way");
    if (fc.browseForFileToSave(true)) {
        //  TODO
    }
}
*/