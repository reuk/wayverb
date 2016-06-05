#include "ModelRendererComponent.hpp"
#include "MoreConversions.hpp"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_inverse.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "combined/config.h"

#include "common/boundaries.h"

ModelRendererComponent::ModelRendererComponent(
    const CopyableSceneData &model,
    model::ValueWrapper<int> &shown_surface,
    model::ValueWrapper<model::App> &app,
    model::ValueWrapper<model::RenderState> &render_state)
        : model(model)
        , shown_surface(shown_surface)
        , app(app)
        , render_state(render_state)
        , scene_renderer(model) {
    set_help("model viewport",
             "This area displays the currently loaded 3D model. Click and drag "
             "to rotate the model, or use the mouse wheel to zoom in and out.");

    open_gl_context.setOpenGLVersionRequired(OpenGLContext::openGL3_2);
    open_gl_context.setRenderer(&scene_renderer);
    open_gl_context.setContinuousRepainting(true);
    open_gl_context.attachTo(*this);
}

ModelRendererComponent::~ModelRendererComponent() noexcept {
    open_gl_context.detach();
}

template <typename T>
static glm::vec2 to_glm_vec2(const T &t) {
    return glm::vec2{t.x, t.y};
}

void ModelRendererComponent::resized() {
    auto bounds = getLocalBounds();
    scene_renderer.set_viewport(glm::vec2{getWidth(), getHeight()});
}

void ModelRendererComponent::mouseDown(const MouseEvent &e) {
    scene_renderer.mouse_down(to_glm_vec2(e.getPosition()));
}

void ModelRendererComponent::mouseDrag(const MouseEvent &e) {
    scene_renderer.mouse_drag(to_glm_vec2(e.getPosition()));
}

void ModelRendererComponent::mouseUp(const juce::MouseEvent &e) {
    scene_renderer.mouse_up(to_glm_vec2(e.getPosition()));
}

void ModelRendererComponent::mouseWheelMove(const MouseEvent &event,
                                            const MouseWheelDetails &wheel) {
    scene_renderer.mouse_wheel_move(wheel.deltaY);
}

static auto get_receiver_directions(
    const model::ValueWrapper<model::App> &app) {
    switch (app.receiver_settings.mode) {
        case model::ReceiverSettings::Mode::microphones: {
            std::vector<glm::vec3> directions(
                app.receiver_settings.microphones.size());
            proc::transform(app.receiver_settings.microphones.get(),
                            directions.begin(),
                            [&app](const auto &i) {
                                return i.pointer.get_pointing(app.receiver);
                            });
            return directions;
        }
        case model::ReceiverSettings::Mode::hrtf: {
            return std::vector<glm::vec3>{
                app.receiver_settings.hrtf.get().get_pointing(app.receiver)};
        }
    }
}

void ModelRendererComponent::receive_broadcast(model::Broadcaster *cb) {
    if (cb == &shown_surface) {
        scene_renderer.set_highlighted(shown_surface);
    } else if (cb == &app.receiver) {
        scene_renderer.set_receiver(app.receiver);
        scene_renderer.set_receiver_pointing(get_receiver_directions(app));
    } else if (cb == &app.source) {
        scene_renderer.set_source(app.source);
    } else if (cb == &render_state.is_rendering) {
        scene_renderer.set_rendering(render_state.is_rendering);
    } else if (cb == &app.receiver_settings) {
        scene_renderer.set_receiver_pointing(get_receiver_directions(app));
    }
}

void ModelRendererComponent::set_positions(
    const std::vector<cl_float3> &positions) {
    scene_renderer.set_positions(positions);
}

void ModelRendererComponent::set_pressures(
    const std::vector<float> &pressures) {
    scene_renderer.set_pressures(pressures);
}

void ModelRendererComponent::changeListenerCallback(ChangeBroadcaster *u) {
    if (u == &scene_renderer) {
        for (auto i : {&shown_connector,
                       &receiver_connector,
                       &source_connector,
                       &is_rendering_connector,
                       &facing_direction_connector}) {
            i->trigger();
        }
    }
}

void ModelRendererComponent::source_dragged(SceneRenderer *,
                                            const glm::vec3 &v) {
    app.source.set(
        glm::clamp(v, model.get_aabb().get_c0(), model.get_aabb().get_c1()));
}
void ModelRendererComponent::receiver_dragged(SceneRenderer *,
                                              const glm::vec3 &v) {
    app.receiver.set(
        glm::clamp(v, model.get_aabb().get_c0(), model.get_aabb().get_c1()));
}