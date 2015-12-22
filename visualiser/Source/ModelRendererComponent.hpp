#pragma once

#include "ModelRenderer.hpp"

class ModelRendererComponent : public Component, public ConfigPanel::Listener {
public:
    ModelRendererComponent();
    virtual ~ModelRendererComponent() noexcept;

    void resized() override;

    void mouseDrag(const MouseEvent & e) override;
    void mouseUp(const MouseEvent & e) override;

    void filesChanged(ConfigPanel * p,
                      const File & object,
                      const File & material,
                      const File & config) override;

private:
    std::unique_ptr<SceneData> sceneData;
    std::unique_ptr<SceneRenderer> sceneRenderer;
    std::unique_ptr<OpenGLContext> openGLContext;
    const float scale{0.01};
    float azimuth{0};
    float elevation{0};
};