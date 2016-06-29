#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class BasicPanel : public Component {
public:
    template <typename T>
    BasicPanel(T&& setup) {
        addAndMakeVisible(property_panel);
        setup(property_panel);
    }

    void resized() override {
        property_panel.setBounds(getLocalBounds());
    }

    auto getTotalContentHeight() const {
        return property_panel.getTotalContentHeight();
    }

private:
    PropertyPanel property_panel;
};
