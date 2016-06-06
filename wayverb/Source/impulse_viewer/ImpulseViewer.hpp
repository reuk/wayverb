#pragma once

#include "ImpulseRenderer.hpp"

class ImpulseViewer : public Component {
public:
    ImpulseViewer();

    void resized() override;

private:
    ImpulseRendererComponent renderer;
    TabbedButtonBar tabs;
};
