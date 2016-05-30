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
    model::ValueWrapper<config::Combined> &config,
    model::ValueWrapper<model::RenderState> &render_state)
        : model(model)
        , shown_surface(shown_surface)
        , config(config)
        , render_state(render_state)
        , scene_renderer(model) {
    set_help("model viewport",
             "This area displays the currently loaded 3D model. Click and drag "
             "to rotate the model, or use the mouse wheel to zoom in and out.");

    openGLContext.setOpenGLVersionRequired(OpenGLContext::openGL3_2);
    openGLContext.setRenderer(&scene_renderer);
    openGLContext.setContinuousRepainting(true);
    openGLContext.attachTo(*this);
}

ModelRendererComponent::~ModelRendererComponent() noexcept {
    openGLContext.detach();
}

void ModelRendererComponent::resized() {
    scene_renderer.set_aspect(getLocalBounds().toFloat().getAspectRatio());
}

void ModelRendererComponent::mouseDrag(const MouseEvent &e) {
    auto p = e.getOffsetFromDragStart().toFloat() * scale;
    auto az = p.x + azimuth;
    auto el = Range<double>(-M_PI / 2, M_PI / 2).clipValue(p.y + elevation);
    scene_renderer.set_rotation(az, el);
}

void ModelRendererComponent::mouseUp(const juce::MouseEvent &e) {
    auto p = e.getOffsetFromDragStart().toFloat() * scale;
    azimuth += p.x;
    elevation += p.y;
}

void ModelRendererComponent::mouseWheelMove(const MouseEvent &event,
                                            const MouseWheelDetails &wheel) {
    scene_renderer.update_scale(wheel.deltaY);
}

void ModelRendererComponent::receive_broadcast(model::Broadcaster *cb) {
    if (cb == &shown_surface) {
        scene_renderer.set_highlighted(shown_surface);
    } else if (cb == &config.mic) {
        scene_renderer.set_mic(config.mic);
    } else if (cb == &config.source) {
        scene_renderer.set_source(config.source);
    } else if (cb == &render_state.is_rendering) {
        scene_renderer.set_rendering(render_state.is_rendering);
    } else if (cb == &config.receiver_config) {
        std::vector<glm::vec3> directions;
        switch (config.receiver_config.mode) {
            case config::AttenuationModel::Mode::microphone:
                proc::transform(
                    config.receiver_config.microphone_model.microphones.get(),
                    std::back_inserter(directions),
                    [](const auto &i) { return to_glm_vec3(i.facing); });
                break;
            case config::AttenuationModel::Mode::hrtf:
                directions.push_back(to_glm_vec3(
                    config.receiver_config.hrtf_model.facing.get()));
                break;
        }
        scene_renderer.set_mic_pointing(directions);
    }
}

void ModelRendererComponent::newOpenGLContextCreated(OpenGLRenderer *r) {
    config.mic.notify();
    config.source.notify();
    render_state.is_rendering.notify();
    config.receiver_config.notify();
}

void ModelRendererComponent::openGLContextClosing(OpenGLRenderer *r) {
}

void ModelRendererComponent::set_positions(
    const std::vector<cl_float3> &positions) {
    scene_renderer.set_positions(positions);
}

void ModelRendererComponent::set_pressures(
    const std::vector<float> &pressures) {
    scene_renderer.set_pressures(pressures);
}
