#pragma once

#include "HelpWindow.hpp"
#include "LeftPanel.hpp"
#include "MeshGeneratorFunctor.hpp"
#include "MeshObject.hpp"
#include "ModelRenderer.hpp"
#include "RenderHelpers.hpp"

class ModelRendererComponent
        : public juce::Component,
          public SettableHelpPanelClient,
          public RendererComponent<SceneRendererContextLifetime>::Listener {
public:
    using Renderer = RendererComponent<SceneRendererContextLifetime>;

    ModelRendererComponent();

    void resized() override;

    void set_scene(wayverb::combined::engine::scene_data scene);

    void set_positions(util::aligned::vector<glm::vec3> positions);
    void set_pressures(util::aligned::vector<float> pressures);
    void set_reflections(util::aligned::vector<util::aligned::vector<wayverb::raytracer::reflection>> reflections,
                      const glm::vec3& source);

    void set_distance_travelled(double distance);

    //  TODO get rid of these somehow?
    void renderer_open_gl_context_created(const Renderer*) override;
    void renderer_open_gl_context_closing(const Renderer*) override;

    void left_panel_debug_show_closest_surfaces(
        wayverb::combined::engine::scene_data scene,
        double sample_rate,
        double speed_of_sound);
    void left_panel_debug_show_boundary_types(
        wayverb::combined::engine::scene_data scene,
        double sample_rate,
        double speed_of_sound);
    void left_panel_debug_hide_debug_mesh();

private:
/*
    void send_highlighted();
    void send_sources();
    void send_receivers();
    void send_is_rendering();
*/

    wayverb::combined::engine::scene_data model;

    Renderer renderer_;
    /*
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
    */
    template <typename T>
    void generate_mesh_async(wayverb::combined::engine::scene_data scene,
                             double sample_rate,
                             double speed_of_sound,
                             T t) {
        generator_ = std::experimental::make_optional(generator_and_connector{});
        generator_->connector = generator_->generator.add_event_finished_callback(
                [this, t = std::move(t)](auto model) {
                    renderer_.context_command([t = std::move(t), m = std::move(model)](auto &i) {
                        t(i, std::move(m));
                    });
                    generator_ = std::experimental::nullopt;
                });
        generator_->generator.run(std::move(scene), sample_rate, speed_of_sound);
    }
    
    struct generator_and_connector final {
        AsyncMeshGenerator generator;
        util::event<wayverb::waveguide::mesh>::scoped_connector connector;
    };
    std::experimental::optional<generator_and_connector> generator_;
};
