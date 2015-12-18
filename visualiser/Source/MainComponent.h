#pragma once

#include "ModelRendererComponent.hpp"

class MainContentComponent final : public Component {
public:
    MainContentComponent();

    void paint(Graphics &);
    void resized();

private:
    std::unique_ptr<ModelRendererComponent> modelRendererComponent;
};
