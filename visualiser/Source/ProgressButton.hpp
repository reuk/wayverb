#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class TransparentTextButton : public TextButton {
public:
    void paintButton(Graphics& g,
                     bool is_mouse_over,
                     bool is_mouse_down) override;

private:
};

class ProgressButton : public Component {
public:
    ProgressButton(double& progress);

    void setColour(int colour_id, Colour new_colour);

    void setClickingTogglesState(bool yes);

    void addListener(Button::Listener* listener);
    void removeListener(Button::Listener* listener);

    void setButtonText(const String& new_text);

    void resized() override;

private:
    juce::ProgressBar bar;
    TransparentTextButton button;
};
