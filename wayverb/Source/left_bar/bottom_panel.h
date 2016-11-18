#pragma once

#include "../UtilityComponents/connector.h"

namespace left_bar {

class bottom : public Component, public TextButton::Listener {
public:
    bottom();

    void paint(Graphics& g) override;
    void resized() override;

    enum class state { idle, rendering };

    //  View methods
    void set_progress(double progress);
    void set_bar_text(const std::string& str);
    void set_state(state s);

private:
    //  Controller methods
    void buttonClicked(Button*) override;

    state state_ = state::idle;
    double progress_ = 0;

    juce::ProgressBar bar_;
    TextButton button_;
    model::Connector<TextButton> button_connector_{&button_, this};
};

}  // namespace left_bar
