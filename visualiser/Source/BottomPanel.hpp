#pragma once

#include "FullModel.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

class BottomPanel : public Component,
                    public model::BroadcastListener,
                    public TextButton::Listener {
public:
    BottomPanel(model::ValueWrapper<model::RenderState>& render_state);

    void paint(Graphics& g) override;
    void resized() override;

    void receive_broadcast(model::Broadcaster* cb) override;

    void buttonClicked(Button*) override;

private:
    double progress{0};
    model::ValueWrapper<model::RenderState>& render_state;
    model::BroadcastConnector is_rendering_connector{&render_state.is_rendering,
                                                     this};
    model::BroadcastConnector state_connector{&render_state.state, this};
    model::BroadcastConnector progress_connector{&render_state.progress, this};

    juce::ProgressBar bar;
    TextButton button;
    model::Connector<TextButton> button_connector{&button, this};
};