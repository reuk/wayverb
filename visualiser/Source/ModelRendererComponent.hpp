#pragma once

#include "ModelRenderer.hpp"
#include "ModelWrapper.hpp"
#include "RenderState.hpp"

class ModelRendererComponent : public Component, public ChangeListener {
public:
    ModelRendererComponent(
        const SceneData& model,
        model::ValueWrapper<config::Combined>& config,
        model::ValueWrapper<model::RenderState>& render_state);
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

    model::ChangeConnector mic_connector{&config.mic, this};
    model::ChangeConnector source_connector{&config.source, this};

    model::ChangeConnector state_connector{&render_state.state, this};
    model::ChangeConnector waveguide_connector{&render_state.show_waveguide,
                                               this};
    model::ChangeConnector raytracer_connector{&render_state.show_raytracer,
                                               this};

    const float scale{0.01};
    float azimuth{0};
    float elevation{0};

    OpenGLContext openGLContext;
    SceneRenderer scene_renderer;
};