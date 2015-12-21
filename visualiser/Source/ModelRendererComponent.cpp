#include "ModelRendererComponent.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

ModelRendererComponent::ModelRendererComponent()
        : modelRenderer(std::make_unique<ModelRenderer>())
        , openGLContext(std::make_unique<OpenGLContext>()) {
    openGLContext->setOpenGLVersionRequired(OpenGLContext::openGL3_2);
    openGLContext->setRenderer(modelRenderer.get());
    openGLContext->setContinuousRepainting(true);
    openGLContext->attachTo(*this);
}

ModelRendererComponent::~ModelRendererComponent() noexcept {
    openGLContext->detach();
}

void ModelRendererComponent::resized() {
    modelRenderer->setAspect(getLocalBounds().toFloat().getAspectRatio());
}

void ModelRendererComponent::mouseDrag(const MouseEvent &e) {
    auto p = e.getOffsetFromDragStart().toFloat() * scale;
    auto az = p.x + azimuth;
    auto el = p.y + elevation;
    auto i = glm::rotate(az, glm::vec3(0, 1, 0));
    auto j = glm::rotate(el, glm::vec3(1, 0, 0));
    modelRenderer->setModelMatrix(j * i);
}

void ModelRendererComponent::mouseUp(const juce::MouseEvent &e) {
    auto p = e.getOffsetFromDragStart().toFloat() * scale;
    azimuth += p.x;
    elevation += p.y;
}

void ModelRendererComponent::filesChanged(ConfigPanel *p,
                                          const File &object,
                                          const File &material,
                                          const File &config) {
    if (object.existsAsFile() && material.existsAsFile() &&
        config.existsAsFile()) {
        sceneData = std::make_unique<SceneData>(
            object.getFullPathName().toStdString(),
            material.getFullPathName().toStdString());
        modelRenderer->setModelObject(*sceneData);
    }
}