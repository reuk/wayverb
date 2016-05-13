#pragma once

#include "BottomPanel.hpp"
#include "ModelRendererComponent.hpp"
#include "ModelWrapper.hpp"
#include "RenderState.hpp"

class RightPanel final : public Component {
public:
    RightPanel(model::Combined& combined_model,
               RenderStateManager& render_state_manager,
               const File& root);

    void resized() override;

    void save_as_project();

private:
    model::Combined& combined_model;
    RenderStateManager& render_state_manager;

    SceneData model;

    ModelRendererComponent model_renderer_component;
};
