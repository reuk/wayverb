#pragma once

#include "vector_list_box.h"

#include "../UtilityComponents/connector.h"

#include "../JuceLibraryCode/JuceHeader.h"

namespace left_bar {

/// Displays a name and a button for further configuration.
template <typename T>
class list_config_item : public updatable_component<T>, public TextButton::Listener {
public:
    using get_callout_component = std::function<std::unique_ptr<Component>(std::shared_ptr<T>)>;

    list_config_item(std::shared_ptr<T> model, get_callout_component get_callout_component)
            : model_{std::move(model)}
            , get_callout_component_{std::move(get_callout_component)} {
        label_.setInterceptsMouseClicks(false, false);

        update(model_);

        this->addAndMakeVisible(label_);
        this->addAndMakeVisible(button_);
    }

    list_config_item(const list_config_item&) = delete;
    list_config_item(list_config_item&&) noexcept = delete;

    list_config_item& operator=(const list_config_item&) = delete;
    list_config_item& operator=(list_config_item&&) noexcept = delete;

    virtual ~list_config_item() noexcept = default;

    void update(std::shared_ptr<T> model) override {
        model_ = model;
        set_label(model_->get_name());
    }

    void set_label(const std::string& label) {
        label_.setText(label, dontSendNotification);
    }

    void resized() override {
        const auto button_width = this->getHeight();
        auto bounds = this->getLocalBounds();
        button_.setBounds(bounds.removeFromRight(button_width).reduced(2, 2));
        label_.setBounds(bounds.reduced(2, 2));
    }

    void buttonClicked(Button*) override {
        CallOutBox::launchAsynchronously(
                get_callout_component_(model_).release(),
                button_.getScreenBounds(),
                nullptr);
    }

private:
    std::shared_ptr<T> model_;
    Label label_;
    TextButton button_{"..."};
    model::Connector<TextButton> button_connector_{&button_, this};

    get_callout_component get_callout_component_;
};

}  // namespace left_bar
