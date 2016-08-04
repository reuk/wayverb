#pragma once

#include "FullModel.hpp"
#include "HelpWindow.hpp"
#include "ModelRenderer.hpp"

#include "OtherComponents/RenderHelpers.hpp"

class ModelRendererComponent : public BaseRendererComponent<SceneRenderer>,
                               public model::BroadcastListener,
                               public ChangeListener,
                               public SettableHelpPanelClient {
public:
    ModelRendererComponent(
            const copyable_scene_data& model,
            model::ValueWrapper<int>& shown_surface,
            model::ValueWrapper<model::App>& app,
            model::ValueWrapper<model::RenderState>& render_state);

    void receive_broadcast(model::Broadcaster* b) override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

private:
    const copyable_scene_data& model;
    model::ValueWrapper<int>& shown_surface;
    model::BroadcastConnector shown_connector{&shown_surface, this};

    model::ValueWrapper<model::App>& app;
    model::ValueWrapper<model::RenderState>& render_state;

    model::BroadcastConnector source_connector{&app.source, this};
    model::BroadcastConnector receiver_settings_connector{
            &app.receiver_settings, this};
    model::BroadcastConnector is_rendering_connector{&render_state.is_rendering,
                                                     this};

    model::BroadcastConnector facing_direction_connector{&app.receiver_settings,
                                                         this};

    model::Connector<ChangeBroadcaster> scene_connector{&renderer, this};
//    model::Connector<SceneRenderer> scene_drag_connector{&renderer, this};
};