#pragma once

#include "ModelRenderer.hpp"

class ModelRendererComponent : public Component {
public:
    ModelRendererComponent();
    virtual ~ModelRendererComponent() noexcept;

    void resized() override;

    void mouseDrag(const MouseEvent & e) override;
    void mouseUp(const MouseEvent & e) override;

private:
    std::unique_ptr<ModelRenderer> modelRenderer;
    std::unique_ptr<OpenGLContext> openGLContext;
    const float scale{0.01};
    float azimuth{0};
    float elevation{0};
};