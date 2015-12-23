#pragma once

#include "ModelRendererComponent.hpp"
#include "ConfigPanel.hpp"

class MainContentComponent final : public Component {
public:
    MainContentComponent();

    void paint(Graphics &);
    void resized();

private:
    std::unique_ptr<ModelRendererComponent> modelRendererComponent;
    //    std::unique_ptr<ConfigPanel> configPanel;
};
