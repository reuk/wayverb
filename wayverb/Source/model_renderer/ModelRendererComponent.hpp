#pragma once

#include "FullModel.hpp"
#include "HelpWindow.hpp"
#include "ModelRenderer.hpp"
#include "RenderHelpers.hpp"

class ModelRendererComponent : public BaseRendererComponent<SceneRenderer>,
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

    void receive_broadcast(model::Broadcaster* b) override;

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

    model::Connector<ChangeBroadcaster> scene_connector{&renderer, this};
    model::Connector<SceneRenderer> scene_drag_connector{&renderer, this};
};