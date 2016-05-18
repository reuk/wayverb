#include "BottomPanel.hpp"

BottomPanel::BottomPanel(model::ValueWrapper<model::RenderState>& render_state)
        : render_state(render_state)
        , progress_button(progress) {
    progress_button.setColour(TextButton::ColourIds::buttonColourId,
                              Colours::green);
    progress_button.setColour(TextButton::ColourIds::buttonOnColourId,
                              Colours::red);

    changeListenerCallback(&render_state.state);
    changeListenerCallback(&render_state.progress);

    addAndMakeVisible(progress_button);
}

void BottomPanel::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
}

void BottomPanel::resized() {
    progress_button.setBounds(getLocalBounds().reduced(2, 2));
}

void BottomPanel::buttonClicked(Button*) {
    render_state.state.set(render_state.state ==
                                   model::RenderState::State::stopped
                               ? model::RenderState::State::started
                               : model::RenderState::State::stopped);
}

void BottomPanel::changeListenerCallback(ChangeBroadcaster* cb) {
    if (cb == &render_state.state) {
        switch (render_state.state) {
            case model::RenderState::State::stopped:
                progress_button.setButtonText("render");
                break;
            case model::RenderState::State::started:
                progress_button.setButtonText("cancel");
                break;
        }
    } else if (cb == &render_state.progress) {
        progress = render_state.progress;
    }
}