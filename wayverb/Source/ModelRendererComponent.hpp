#pragma once

#include "FullModel.hpp"
#include "HelpWindow.hpp"
#include "ModelRenderer.hpp"

class ModelRendererComponent : public Component,
                               public model::BroadcastListener,
                               public SceneRenderer::Listener,
                               public ChangeListener,
                               public SettableHelpPanelClient {
public:
    ModelRendererComponent(
        const CopyableSceneData& model,
        model::ValueWrapper<int>& shown_surface,
        model::ValueWrapper<model::App>& app,
        model::ValueWrapper<model::RenderState>& render_state);
    virtual ~ModelRendererComponent() noexcept;

    void resized() override;

    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    void mouseWheelMove(const MouseEvent& event,
                        const MouseWheelDetails& wheel) override;

    void receive_broadcast(model::Broadcaster* b) override;

    void set_positions(const std::vector<cl_float3>& positions);
    void set_pressures(const std::vector<float>& pressures);

    void source_dragged(SceneRenderer*, const glm::vec3& v) override;
    void receiver_dragged(SceneRenderer*, const glm::vec3& v) override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

private:
    const CopyableSceneData& model;
    model::ValueWrapper<int>& shown_surface;
    model::BroadcastConnector shown_connector{&shown_surface, this};

    model::ValueWrapper<model::App>& app;
    model::ValueWrapper<model::RenderState>& render_state;

    model::BroadcastConnector receiver_connector{&app.receiver, this};
    model::BroadcastConnector source_connector{&app.source, this};
    model::BroadcastConnector is_rendering_connector{&render_state.is_rendering,
                                                     this};

    model::BroadcastConnector facing_direction_connector{&app.receiver_settings,
                                                         this};

    OpenGLContext open_gl_context;
    SceneRenderer scene_renderer;
    model::Connector<ChangeBroadcaster> scene_connector{&scene_renderer, this};
    model::Connector<SceneRenderer> scene_drag_connector{&scene_renderer, this};
};