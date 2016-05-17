#include "BottomPanel.hpp"

BottomPanel::BottomPanel(
    model::ValueWrapper<model::RenderStateManager>& render_state_manager)
        : render_state_manager(render_state_manager)
        , progress_button(progress) {
    progress_button.setColour(TextButton::ColourIds::buttonColourId,
                              Colours::green);
    progress_button.setColour(TextButton::ColourIds::buttonOnColourId,
                              Colours::red);

    changeListenerCallback(&render_state_manager.state);
    changeListenerCallback(&render_state_manager.progress);

    addAndMakeVisible(progress_button);
}

void BottomPanel::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
}

void BottomPanel::resized() {
    progress_button.setBounds(getLocalBounds().reduced(2, 2));
}

void BottomPanel::buttonClicked(Button*) {
    render_state_manager.state.set_value(render_state_manager.state ==
                                                 model::RenderState::stopped
                                             ? model::RenderState::started
                                             : model::RenderState::stopped);
}

void BottomPanel::changeListenerCallback(ChangeBroadcaster* cb) {
    if (cb == &render_state_manager.state) {
        switch (render_state_manager.state) {
            case model::RenderState::stopped:
                progress_button.setButtonText("render");
                break;
            case model::RenderState::started:
                progress_button.setButtonText("cancel");
                break;
        }
    } else if (cb == &render_state_manager.progress) {
        progress = render_state_manager.progress;
    }
}