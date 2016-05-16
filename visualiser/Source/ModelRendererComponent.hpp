#pragma once

#include "ModelRenderer.hpp"
#include "ModelWrapper.hpp"
#include "RenderState.hpp"

class ModelRendererComponent : public Component,
                               public RenderStateManager::Listener {
public:
    ModelRendererComponent(RenderStateManager& render_state_manager,
                           const SceneData& model,
                           const config::Combined& config);
    virtual ~ModelRendererComponent() noexcept;

    void resized() override;

    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    void mouseWheelMove(const MouseEvent& event,
                        const MouseWheelDetails& wheel) override;

    void render_state_changed(RenderStateManager*, RenderState state) override;
    void render_progress_changed(RenderStateManager*, double progress) override;

private:
    RenderStateManager& render_state_manager;
    model::Connector<RenderStateManager> render_state_connector{
        &render_state_manager, this};

    const SceneData& model;
    const config::Combined& config;

    const float scale{0.01};
    float azimuth{0};
    float elevation{0};

    OpenGLContext openGLContext;
    SceneRenderer scene_renderer;
};