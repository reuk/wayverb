#pragma once

#include "ConfigPanel.hpp"
#include "ModelRendererComponent.hpp"

class MainContentComponent final : public Component {
public:
    MainContentComponent(const File& root);

    void paint(Graphics&) override;
    void resized() override;

    void save_as_project();

    template <typename Protocol>
    void project_read_write(Protocol& protocol) {
        protocol("model.model", model);
        protocol("materials.json", materials);
        protocol("config.json", config);
    }

    SceneData model;
    SurfaceConfig materials;
    config::Combined config;

    ModelRendererComponent modelRendererComponent;
};
