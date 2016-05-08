#include "BottomPanel.hpp"

BottomPanel::BottomPanel(RenderStateManager& render_state_manager)
        : render_state_manager(render_state_manager)
        //        , render_button() , progress_bar(progress)
        , progress_button(progress) {
    //    render_button.setClickingTogglesState(true);
    //    render_button.setColour(TextButton::ColourIds::buttonColourId,
    //                            Colours::green);
    //    render_button.setColour(TextButton::ColourIds::buttonOnColourId,
    //                            Colours::red);
    //
    //    progress_bar.setPercentageDisplay(false);
    //
    //    render_button.addListener(this);
    //    addAndMakeVisible(render_button);
    //    addAndMakeVisible(progress_bar);

    progress_button.setColour(TextButton::ColourIds::buttonColourId,
                              Colours::green);
    progress_button.setColour(TextButton::ColourIds::buttonOnColourId,
                              Colours::red);

    progress_button.addListener(this);
    addAndMakeVisible(progress_button);

    render_state_manager.add_listener(*this);
}

BottomPanel::~BottomPanel() {
    render_state_manager.remove_listener(*this);
}

void BottomPanel::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
}

void BottomPanel::resized() {
    //    auto panel_bounds = getLocalBounds();
    //    auto padding = 2;
    //    auto padded_bounds = getLocalBounds().reduced(padding);
    //
    //    render_button.setBounds(padded_bounds.removeFromRight(100));
    //    progress_bar.setBounds(padded_bounds.withTrimmedRight(padding));

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