#include "RightPanel.hpp"

#include "LoadFiles.hpp"

RightPanel::RightPanel(model::Combined& combined_model,
                       RenderStateManager& render_state_manager,
                       const File& root)
        : combined_model(combined_model)
        , render_state_manager(render_state_manager)
        , model(load_model(root, load_materials(root)))
        , model_renderer_component(
              render_state_manager, model, combined_model.get_data()) {
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
