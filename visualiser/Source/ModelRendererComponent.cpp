#include "ModelRendererComponent.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "combined/config.h"

#include "common/boundaries.h"

ModelRendererComponent::ModelRendererComponent(
    const SceneData &model,
    model::ValueWrapper<config::Combined> &config,
    model::ValueWrapper<model::RenderState> &render_state)
        : model(model)
        , config(config)
        , render_state(render_state)
        , scene_renderer(model) {
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
    if (cb == &config.mic) {
        scene_renderer.set_mic(config.mic);
    } else if (cb == &config.source) {
        scene_renderer.set_source(config.source);
    } else if (cb == &render_state.is_rendering) {
        scene_renderer.set_rendering(render_state.is_rendering);
    }
}

void ModelRendererComponent::newOpenGLContextCreated(OpenGLRenderer *r) {
    config.mic.notify();
    config.source.notify();
    render_state.is_rendering.notify();
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