#pragma once

#include "FullModel.hpp"
#include "HelpWindow.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

class PolarPatternDisplay : public Component,
                            public model::BroadcastListener,
                            public SettableHelpPanelClient {
public:
    PolarPatternDisplay(model::ValueWrapper<float>& shape);

    void paint(Graphics& g) override;

    void receive_broadcast(model::Broadcaster* b) override;

private:
    model::ValueWrapper<float>& shape;
    model::BroadcastConnector shape_connector{&shape, this};
};
