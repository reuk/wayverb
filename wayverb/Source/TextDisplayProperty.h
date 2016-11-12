#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include <sstream>

class TextDisplayProperty : public PropertyComponent {
public:
    TextDisplayProperty(const String& name, int height)
            : PropertyComponent{name, height} {
        addAndMakeVisible(label_);
    }

    void refresh() override {}

    template <typename T>
    void display(const T& t) {
        std::stringstream ss;
        ss << t;
        label_.setText(ss.str(), dontSendNotification);
    }

private:
    Label label_;
};
