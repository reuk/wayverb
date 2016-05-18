#pragma once

#include "ModelWrapper.hpp"
#include "ProgressButton.hpp"
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
    model::ChangeConnector state_connector{&render_state.state, this};
    model::ChangeConnector progress_connector{&render_state.progress, this};

    ProgressButton progress_button;
    model::Connector<ProgressButton> button_connector{&progress_button, this};
};