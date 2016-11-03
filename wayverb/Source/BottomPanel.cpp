#include "BottomPanel.hpp"

BottomPanel::BottomPanel()
        : bar_{progress_} {
    addAndMakeVisible(bar_);
    addAndMakeVisible(button_);
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
    switch (s) {
        case state::idle: {
            button_.setButtonText("render");
            break;
        }

        case state::rendering: {
            button_.setButtonText("cancel");
            break;
        }
    };
}

//  Controller methods
void BottomPanel::buttonClicked(Button* b) {
    //  TODO
}
