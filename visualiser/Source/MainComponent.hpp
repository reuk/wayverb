#pragma once

#include "ModelRendererComponent.hpp"
#include "ConfigPanel.hpp"
#include "DemoPanel.hpp"

class MainContentComponent final : public Component {
public:
    MainContentComponent();

    void paint(Graphics &) override;
    void resized() override;

private:
    std::unique_ptr<ModelRendererComponent> modelRendererComponent;
    std::unique_ptr<DemoPanel> demo_panel;
};
