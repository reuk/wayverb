#pragma once

#include "ModelRenderer.hpp"

class ModelRendererComponent : public Component {
public:
    ModelRendererComponent();
    virtual ~ModelRendererComponent() noexcept;

    void resized() override;

private:
    std::unique_ptr<ModelRenderer> modelRenderer;
    std::unique_ptr<OpenGLContext> openGLContext;
};