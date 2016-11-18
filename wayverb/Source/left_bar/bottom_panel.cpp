#include "bottom_panel.h"
#include "Application.h"
#include "CommandIDs.h"

namespace left_bar {

bottom::bottom()
        : bar_{progress_} {
    addAndMakeVisible(bar_);
    addAndMakeVisible(button_);

    set_state(state::idle);
}

void bottom::paint(Graphics& g) { g.fillAll(Colours::darkgrey); }

void bottom::resized() {
    const auto button_width = 100;
    auto bounds = getLocalBounds().reduced(2, 2);

    bar_.setBounds(bounds.withTrimmedRight(button_width + 1));
    button_.setBounds(bounds.removeFromRight(button_width - 1));
}

//  View methods
void bottom::set_progress(double progress) { progress_ = progress; }
void bottom::set_bar_text(const std::string& str) {
    bar_.setTextToDisplay(str.c_str());
}
void bottom::set_state(state s) {
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
void bottom::buttonClicked(Button*) {
    wayverb_application::get_command_manager().invokeDirectly(
            state_ == state::idle ? CommandIDs::idStartRender
                                  : CommandIDs::idCancelRender,
            true);
}

}  // namespace left_bar
