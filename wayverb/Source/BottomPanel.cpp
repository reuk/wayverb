#include "BottomPanel.h"
#include "Application.h"
#include "CommandIDs.h"

BottomPanel::BottomPanel()
        : bar_{progress_} {
    addAndMakeVisible(bar_);
    addAndMakeVisible(button_);

    set_state(state::idle);
}

void BottomPanel::paint(Graphics& g) { g.fillAll(Colours::darkgrey); }

void BottomPanel::resized() {
    const auto button_width = 100;
    auto bounds = getLocalBounds().reduced(2, 2);

    bar_.setBounds(bounds.withTrimmedRight(button_width + 1));
    button_.setBounds(bounds.removeFromRight(button_width - 1));
}

//  View methods
void BottomPanel::set_progress(double progress) { progress_ = progress; }
void BottomPanel::set_bar_text(const std::string& str) {
    bar_.setTextToDisplay(str.c_str());
}
void BottomPanel::set_state(state s) {
    state_ = s;
    switch (s) {
        case state::idle: {
            button_.setButtonText("render");
            bar_.setTextToDisplay("");
            progress_ = 0;
            break;
        }

        case state::rendering: {
            button_.setButtonText("cancel");
            break;
        }
    };
}

//  Controller methods
void BottomPanel::buttonClicked(Button*) {
    wayverb_application::get_command_manager().invokeDirectly(
            state_ == state::idle ? CommandIDs::idStartRender
                                  : CommandIDs::idCancelRender,
            true);
}
