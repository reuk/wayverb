#pragma once

#include "BottomPanel.hpp"
#include "ModelRendererComponent.hpp"
#include "RenderState.hpp"

class RightPanel final : public Component {
public:
    RightPanel(RenderStateManager& render_state_manager, const File& root);

    void resized() override;

    void save_as_project();

private:
    RenderStateManager& render_state_manager;
    
    SceneData model;
    config::Combined config;

    ModelRendererComponent model_renderer_component;
};
