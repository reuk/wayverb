#include "ModelRendererComponent.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "combined_config.h"

#include "common/boundaries.h"

ModelRendererComponent::ModelRendererComponent(const SceneData &model,
                                               const config::Combined &config)
        : model(model)
        , config(config)
        , scene_renderer(model, config) {
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
    auto el = p.y + elevation;
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