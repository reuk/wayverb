
#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace left_bar {

template <typename T>
class heading_component final : public Component {
public:
    template <typename... Ts>
    heading_component(const std::string& title, Ts&&... ts)
            : content{std::forward<Ts>(ts)...}
            , label_{"", title} {
        addAndMakeVisible(content);
        addAndMakeVisible(label_);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        label_.setBounds(bounds.removeFromTop(20));
        content.setBounds(bounds);
    }

    T content;

private:
    Label label_;
};

}  // namespace left_bar
