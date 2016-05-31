#pragma once

#include "FullModel.hpp"
#include "HelpWindow.hpp"
#include "ModelRenderer.hpp"

class ModelRendererComponent : public Component,
                               public model::BroadcastListener,
                               public ChangeListener,
                               public SettableHelpPanelClient {
public:
    ModelRendererComponent(
        const CopyableSceneData& model,
        model::ValueWrapper<int>& shown_surface,
        model::ValueWrapper<config::Combined>& config,
        model::ValueWrapper<model::RenderState>& render_state);
    virtual ~ModelRendererComponent() noexcept;

    void resized() override;

    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    void mouseWheelMove(const MouseEvent& event,
                        const MouseWheelDetails& wheel) override;

    void receive_broadcast(model::Broadcaster* b) override;

    void set_positions(const std::vector<cl_float3>& positions);
    void set_pressures(const std::vector<float>& pressures);

    void changeListenerCallback(ChangeBroadcaster* cb) override;

private:
    const CopyableSceneData& model;
    model::ValueWrapper<int>& shown_surface;
    model::BroadcastConnector shown_connector{&shown_surface, this};

    model::ValueWrapper<config::Combined>& config;
    model::ValueWrapper<model::RenderState>& render_state;

    model::BroadcastConnector mic_connector{&config.mic, this};
    model::BroadcastConnector source_connector{&config.source, this};
    model::BroadcastConnector is_rendering_connector{&render_state.is_rendering,
                                                     this};

    model::BroadcastConnector facing_direction_connector{
        &config.receiver_config, this};

    const float scale{0.01};
    float azimuth{0};
    float elevation{0};

    OpenGLContext openGLContext;
    SceneRenderer scene_renderer;
    model::Connector<ChangeBroadcaster> scene_connector{&scene_renderer, this};
};