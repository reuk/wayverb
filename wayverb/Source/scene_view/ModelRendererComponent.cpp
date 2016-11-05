#include "ModelRendererComponent.hpp"
#include "Application.hpp"
#include "CommandIDs.hpp"

#include "../model/model.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <experimental/optional>

ModelRendererComponent::ModelRendererComponent()
        : model{std::move(model)}
        , renderer_{[=] { return SceneRendererContextLifetime{}; }}
        {
    set_help("model viewport",
             "This area displays the currently loaded 3D model. Click and drag "
             "to rotate the model, or use the mouse wheel to zoom in and out.");
    addAndMakeVisible(renderer_);
}

void ModelRendererComponent::resized() { renderer_.setBounds(getLocalBounds()); }

void ModelRendererComponent::set_scene(wayverb::combined::engine::scene_data scene) {
    renderer_.context_command([scene = std::move(scene)] (auto& i) { i.set_scene(std::move(scene)); });
}

void ModelRendererComponent::set_positions(util::aligned::vector<glm::vec3> positions) {
    renderer_.context_command([positions = std::move(positions)](auto &i) { i.set_positions(std::move(positions)); });
}

void ModelRendererComponent::set_pressures(util::aligned::vector<float> pressures) {
    renderer_.context_command([pressures = std::move(pressures)](auto &i) { i.set_pressures(std::move(pressures)); });
}

void ModelRendererComponent::set_reflections( util::aligned::vector<util::aligned::vector<wayverb::raytracer::reflection>> reflections, const glm::vec3 &source) {
    renderer_.context_command([reflections=std::move(reflections), source](auto &i) { i.set_reflections(reflections, source); });
}

void ModelRendererComponent::set_distance_travelled(double distance) {
    renderer_.context_command([=](auto& i) {i.set_distance_travelled(distance);});
}

/*
void ModelRendererComponent::send_highlighted() {
    renderer.context_command([s = shown_surface.get()](auto &i) {
        i.set_highlighted(s);
    });
}

void ModelRendererComponent::send_sources() {
    renderer.context_command([s = app.source.get()](auto &i) {
        i.set_sources(s);
    });
}

void ModelRendererComponent::send_receivers() {
    renderer.context_command([s = app.receiver_settings.get()](auto &i) {
        i.set_receivers(s);
    });
}

void ModelRendererComponent::send_is_rendering() {
    renderer.context_command([s = render_state.is_rendering.get()](auto &i) {
        i.set_rendering(s);
    });
}
*/

void ModelRendererComponent::renderer_open_gl_context_created(
        const Renderer *r) {
    /*
    if (r == &renderer) {
        send_highlighted();
        send_sources();
        send_receivers();
        send_is_rendering();
    }
    */
}

void ModelRendererComponent::renderer_open_gl_context_closing(
        const Renderer *r) {
    //  don't care
}

void ModelRendererComponent::left_panel_debug_show_closest_surfaces(
        wayverb::combined::engine::scene_data scene,
        double sample_rate,
        double speed_of_sound) {
    generator_ = std::experimental::make_optional(generator_and_connector{});

    generator_->connector = generator_->generator.add_event_finished_callback(
            [&](auto model) {
                renderer_.context_command([m = std::move(model)](auto &i) {
                    i.debug_show_closest_surfaces(std::move(m));
                });
                generator_ = std::experimental::nullopt;
            });

    generator_->generator.run(std::move(scene), sample_rate, speed_of_sound);
}

void ModelRendererComponent::left_panel_debug_show_boundary_types(
        wayverb::combined::engine::scene_data scene,
        double sample_rate,
        double speed_of_sound) {
    generator_ = std::experimental::make_optional(generator_and_connector{});

    generator_->connector = generator_->generator.add_event_finished_callback(
            [&](auto model) {
                renderer_.context_command([m = std::move(model)](auto &i) {
                    i.debug_show_boundary_types(std::move(m));
                });
                generator_ = std::experimental::nullopt;
            });
    
    generator_->generator.run(std::move(scene), sample_rate, speed_of_sound);
}

void ModelRendererComponent::left_panel_debug_hide_debug_mesh() {
    renderer_.context_command([](auto &i) { i.debug_hide_model(); });
}
