#include "ModelRendererComponent.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "boundaries.h"
#include "combined_config.h"

ModelRendererComponent::ModelRendererComponent()
        : sceneRenderer(std::make_unique<SceneRenderer>())
        , openGLContext(std::make_unique<OpenGLContext>()) {
    openGLContext->setOpenGLVersionRequired(OpenGLContext::openGL3_2);
    openGLContext->setRenderer(sceneRenderer.get());
    openGLContext->setContinuousRepainting(true);
    openGLContext->attachTo(*this);
}

ModelRendererComponent::~ModelRendererComponent() noexcept {
    openGLContext->detach();
}

void ModelRendererComponent::resized() {
    sceneRenderer->set_aspect(getLocalBounds().toFloat().getAspectRatio());
}

void ModelRendererComponent::mouseDrag(const MouseEvent &e) {
    auto p = e.getOffsetFromDragStart().toFloat() * scale;
    auto az = p.x + azimuth;
    auto el = p.y + elevation;
    sceneRenderer->set_rotation(az, el);
}

void ModelRendererComponent::mouseUp(const juce::MouseEvent &e) {
    auto p = e.getOffsetFromDragStart().toFloat() * scale;
    azimuth += p.x;
    elevation += p.y;
}

void ModelRendererComponent::file_package_loaded(DemoPanel &,
                                                 const FilePackage &fp) {
    sceneRenderer->load_from_file_package(fp);
}

void ModelRendererComponent::mouseWheelMove(const MouseEvent &event,
                                            const MouseWheelDetails &wheel) {
    sceneRenderer->update_scale(wheel.deltaY);
}