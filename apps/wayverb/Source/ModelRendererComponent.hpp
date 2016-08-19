#pragma once

#include "FullModel.hpp"
#include "HelpWindow.hpp"
#include "LeftPanel.hpp"
#include "MeshGeneratorFunctor.hpp"
#include "MeshObject.hpp"
#include "ModelRenderer.hpp"

#include "OtherComponents/RenderHelpers.hpp"

class ModelRendererComponent
        : public juce::Component,
          public model::BroadcastListener,
          public SettableHelpPanelClient,
          public LeftPanel::Listener,
          public RendererComponent<SceneRendererContextLifetime>::Listener {
public:
    using Renderer = RendererComponent<SceneRendererContextLifetime>;

    ModelRendererComponent(
            const copyable_scene_data& model,
            model::ValueWrapper<int>& shown_surface,
            model::ValueWrapper<model::App>& app,
            model::ValueWrapper<model::RenderState>& render_state);

    void resized() override;

    void receive_broadcast(model::Broadcaster* b) override;

    void set_positions(const aligned::vector<glm::vec3>& positions);
    void set_pressures(const aligned::vector<float>& pressures,
                       float current_time);
    void set_impulses(const aligned::vector<aligned::vector<impulse>>& impulses,
                      const glm::vec3& source,
                      const glm::vec3& receiver);

    void renderer_open_gl_context_created(const Renderer*) override;
    void renderer_open_gl_context_closing(const Renderer*) override;

    void left_panel_debug_show_closest_surfaces(const LeftPanel*) override;
    void left_panel_debug_show_boundary_types(const LeftPanel*) override;
    void left_panel_debug_hide_debug_mesh(const LeftPanel*) override;

private:
    void send_highlighted();
    void send_sources();
    void send_receivers();
    void send_is_rendering();

    const copyable_scene_data& model;
    model::ValueWrapper<int>& shown_surface;

    class MeshGenerator final : public AsyncMeshGenerator::Listener {
    public:
        MeshGenerator(const copyable_scene_data& scene,
                      double sample_rate,
                      std::function<void(waveguide::mesh::model)>
                              on_finished);

        void async_mesh_generator_finished(
                const AsyncMeshGenerator*,
                waveguide::mesh::model model) override;

    private:
        std::function<void(waveguide::mesh::model)> on_finished;
        AsyncMeshGenerator generator;
        model::Connector<AsyncMeshGenerator> generator_connector{&generator,
                                                                 this};
    };

    Renderer renderer;
    model::Connector<Renderer> renderer_connector{&renderer, this};

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

    std::unique_ptr<MeshGenerator> generator;

    //    model::Connector<SceneRenderer> scene_drag_connector{&renderer, this};
};