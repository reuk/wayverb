#pragma once

#include "RenderState.hpp"

class BottomPanel : public Component,
                    public ChangeListener,
                    public TextButton::Listener {
public:
    BottomPanel(model::ValueWrapper<model::RenderState>& render_state);

    void paint(Graphics& g) override;
    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

    void buttonClicked(Button*) override;

private:
    double progress{0};
    model::ValueWrapper<model::RenderState>& render_state;
    model::ChangeConnector is_rendering_connector{&render_state.is_rendering,
                                                  this};
    model::ChangeConnector state_connector{&render_state.state, this};
    model::ChangeConnector progress_connector{&render_state.progress, this};

    juce::ProgressBar bar;
    TextButton button;
    model::Connector<TextButton> button_connector{&button, this};
};