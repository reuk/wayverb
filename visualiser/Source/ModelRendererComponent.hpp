#pragma once

#include "DemoPanel.hpp"
#include "ModelRenderer.hpp"

class ModelRendererComponent : public Component, public DemoPanel::Listener {
public:
    ModelRendererComponent();
    virtual ~ModelRendererComponent() noexcept;

    void resized() override;

    void mouseDrag(const MouseEvent& e) override;
    void mouseUp(const MouseEvent& e) override;

    void mouseWheelMove(const MouseEvent& event,
                        const MouseWheelDetails& wheel) override;

    void file_package_loaded(DemoPanel&, const FilePackage& fp) override;

private:
    std::unique_ptr<SceneData> sceneData;
    std::unique_ptr<SceneRenderer> sceneRenderer;
    std::unique_ptr<OpenGLContext> openGLContext;
    const float scale{0.01};
    float azimuth{0};
    float elevation{0};
};