#pragma once

#include "DemoPanel.hpp"
#include "ModelRenderer.hpp"

class ModelRendererComponent : public Component {
public:
    ModelRendererComponent(const SceneData& model,
                           const config::Combined& config);
    virtual ~ModelRendererComponent() noexcept;

    void resized() override;

    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    void mouseWheelMove(const MouseEvent& event,
                        const MouseWheelDetails& wheel) override;

private:
    const SceneData& model;
    const config::Combined& config;

    SceneRenderer scene_renderer;
    OpenGLContext openGLContext;

    const float scale{0.01};
    float azimuth{0};
    float elevation{0};
};