#pragma once

#include "connector.hpp"

#include "../JuceLibraryCode/JuceHeader.h"

class BottomPanel : public Component,
                    public model::BroadcastListener,
                    public TextButton::Listener {
public:
    BottomPanel();

    void paint(Graphics& g) override;
    void resized() override;

    enum class state { idle, rendering };

    //  View methods
    void set_progress(double progress);
    void set_bar_text(const std::string& str);
    void set_state(state s);

    //  Controller methods
    void buttonClicked(Button*) override;

private:
    double progress_{0};

    juce::ProgressBar bar_;
    TextButton button_;
    model::Connector<TextButton> button_connector_{&button_, this};
};
