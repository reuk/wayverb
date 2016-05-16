#include "BottomPanel.hpp"

BottomPanel::BottomPanel(RenderStateManager& render_state_manager)
        : render_state_manager(render_state_manager)
        , progress_button(progress) {
    progress_button.setColour(TextButton::ColourIds::buttonColourId,
                              Colours::green);
    progress_button.setColour(TextButton::ColourIds::buttonOnColourId,
                              Colours::red);
    addAndMakeVisible(progress_button);
}

void BottomPanel::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
}

void BottomPanel::resized() {
    progress_button.setBounds(getLocalBounds().reduced(2, 2));
}

void BottomPanel::render_state_changed(RenderStateManager*, RenderState state) {
    switch (state) {
        case RenderState::stopped:
            progress_button.setButtonText("render");
            break;
        case RenderState::started:
            progress_button.setButtonText("cancel");
            break;
    }
}

void BottomPanel::render_progress_changed(RenderStateManager*, double p) {
    progress = p;
}

void BottomPanel::buttonClicked(Button*) {
    render_state_manager.set_state(render_state_manager.get_state() ==
                                           RenderState::stopped
                                       ? RenderState::started
                                       : RenderState::stopped);
}