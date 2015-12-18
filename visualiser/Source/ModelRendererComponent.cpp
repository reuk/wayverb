#include "ModelRendererComponent.hpp"

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