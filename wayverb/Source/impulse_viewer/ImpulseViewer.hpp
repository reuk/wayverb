#pragma once

#include "ImpulseRenderer.hpp"
#include "FullModel.hpp"

class ImpulseViewer : public Component , public ChangeListener {
public:
    ImpulseViewer(const File& file);

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

private:
    ImpulseRendererComponent renderer;
    TabbedButtonBar tabs;

    model::Connector<ChangeBroadcaster> tabs_connector{&tabs, this};
};
