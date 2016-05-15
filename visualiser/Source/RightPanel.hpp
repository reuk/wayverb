#pragma once

#include "BottomPanel.hpp"
#include "ModelRendererComponent.hpp"
#include "ModelWrapper.hpp"
#include "RenderState.hpp"

class RightPanel final : public Component {
public:
    RightPanel(model::Combined& combined_model,
               SceneData& scene_data,
               RenderStateManager& render_state_manager);

    void resized() override;

    void save_as_project();

private:
    model::Combined& combined_model;
    SceneData& scene_data;
    RenderStateManager& render_state_manager;

    ModelRendererComponent model_renderer_component;
};
