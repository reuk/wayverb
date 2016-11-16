#pragma once

#include "../UtilityComponents/connector.h"

#include "../JuceLibraryCode/JuceHeader.h"

namespace left_bar {

/// Displays a name and a button for further configuration.
template <typename T>
class list_config_item : public Component, public TextButton::Listener {
public:
    list_config_item() {
        label_.setInterceptsMouseClicks(false, false);

        addAndMakeVisible(label_);
        addAndMakeVisible(button_);
    }

    list_config_item(const list_config_item&) = delete;
    list_config_item(list_config_item&&) noexcept = delete;

    list_config_item& operator=(const list_config_item&) = delete;
    list_config_item& operator=(list_config_item&&) noexcept = delete;

    virtual ~list_config_item() noexcept = default;

    void update(T& model) {
        model_ = &model;
        set_label(model_->get_name());
    }

    void set_label(const std::string& label) {
        label_.setText(label, dontSendNotification);
    }

    void resized() override {
        const auto button_width = getHeight();
        auto bounds = getLocalBounds();
        button_.setBounds(bounds.removeFromRight(button_width).reduced(2, 2));
        label_.setBounds(bounds.reduced(2, 2));
    }

    void buttonClicked(Button*) override {
        CallOutBox::launchAsynchronously(
                get_callout_component(*model_).release(),
                button_.getScreenBounds(),
                nullptr);
    }

private:
    virtual std::unique_ptr<Component> get_callout_component(T& model) = 0;

    T* model_ = nullptr;
    Label label_;
    TextButton button_{"..."};
    model::Connector<TextButton> button_connector_{&button_, this};
};

}  // namespace left_bar
