#include "ProgressButton.h"

void TransparentTextButton::paintButton(Graphics& g,
                                        bool is_mouse_over,
                                        bool is_mouse_down) {
    auto bounds = getLocalBounds().reduced(1, 1);
    bounds.removeFromBottom(1);

    Colour c = Colours::transparentWhite;
    if (is_mouse_over) {
        c = Colours::white.withAlpha(0.1f);
    }
    if (is_mouse_down) {
        c = Colours::black.withAlpha(0.1f);
    }

    g.setColour(c);
    g.fillRect(bounds);

    LookAndFeel& lf = getLookAndFeel();
    lf.drawButtonText(g, *this, is_mouse_over, is_mouse_down);
}

ProgressButton::ProgressButton(double& progress)
        : bar(progress) {
    button.setClickingTogglesState(true);
    bar.setPercentageDisplay(false);

    addAndMakeVisible(bar);
    addAndMakeVisible(button);
}

void ProgressButton::setColour(int colour_id, Colour new_colour) {
    bar.setColour(colour_id, new_colour);
    //    button.setColour(colour_id, new_colour);
}

void ProgressButton::addListener(Button::Listener* listener) {
    button.addListener(listener);
}
void ProgressButton::removeListener(Button::Listener* listener) {
    button.removeListener(listener);
}

void ProgressButton::setButtonText(const String& new_text) {
    button.setButtonText(new_text);
}

void ProgressButton::resized() {
    bar.setBounds(getLocalBounds());
    button.setBounds(getLocalBounds());
}
