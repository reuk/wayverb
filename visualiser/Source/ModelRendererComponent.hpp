#pragma once

#include "ModelRenderer.hpp"
#include "ModelWrapper.hpp"
#include "RenderState.hpp"

class ModelRendererComponent : public Component, public ChangeListener {
public:
    ModelRendererComponent(
        const SceneData& model,
        model::ValueWrapper<config::Combined>& config,
        model::ValueWrapper<model::RenderStateManager>& render_state_manager);
    virtual ~ModelRendererComponent() noexcept;

    void resized() override;

    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    void mouseWheelMove(const MouseEvent& event,
                        const MouseWheelDetails& wheel) override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

private:
    const SceneData& model;
    model::ValueWrapper<config::Combined>& config;

    model::ValueWrapper<model::RenderState>& render_state;
    model::ChangeConnector render_state_connector{&render_state, this};

    const float scale{0.01};
    float azimuth{0};
    float elevation{0};

    OpenGLContext openGLContext;
    SceneRenderer scene_renderer;
};