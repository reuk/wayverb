#pragma once

#include "AngularLookAndFeel.h"
#include "connector.h"

template <typename T>
class ok_cancel_window final : public Component, public ButtonListener {
public:
    ok_cancel_window(std::unique_ptr<T> content)
            : content_{std::move(content)} {
        done_.setColour(TextButton::ColourIds::buttonColourId,
                        AngularLookAndFeel::emphasis);

        addAndMakeVisible(done_);
        done_.setExplicitFocusOrder(1);

        addAndMakeVisible(cancel_);
        done_.setExplicitFocusOrder(2);

        addAndMakeVisible(*content_);
        done_.setExplicitFocusOrder(3);

        setSize(content_->getWidth(),
                content_->getHeight() + button_bar_height_);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        auto button_bounds = bounds.removeFromBottom(button_bar_height_);
        content_->setBounds(bounds);

        done_.setBounds(button_bounds.removeFromRight(100).reduced(2, 2));
        cancel_.setBounds(button_bounds.removeFromRight(100).reduced(2, 2));
    }

    void buttonClicked(Button* b) override {
        if (auto dw = findParentComponentOfClass<DialogWindow>()) {
            if (b == &done_) {
                dw->exitModalState(1);
            } else if (b == &cancel_) {
                dw->exitModalState(0);
            }
        }
    }

private:
    static constexpr auto button_bar_height_ = 25;

    std::unique_ptr<T> content_;

    TextButton done_{"OK"};
    model::Connector<TextButton> done_connector_{&done_, this};

    TextButton cancel_{"cancel"};
    model::Connector<TextButton> cancel_connector_{&cancel_, this};
};

template <typename T>
auto make_ok_cancel_window_ptr(std::unique_ptr<T> ptr) {
    return std::make_unique<ok_cancel_window<T>>(std::move(ptr));
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class done_window final : public Component, public ButtonListener {
public:
    done_window(std::unique_ptr<T> content)
            : content_{std::move(content)} {
        done_.setColour(TextButton::ColourIds::buttonColourId,
                        AngularLookAndFeel::emphasis);

        addAndMakeVisible(done_);
        done_.setExplicitFocusOrder(1);

        addAndMakeVisible(*content_);
        content_->setExplicitFocusOrder(2);

        setSize(content_->getWidth(),
                content_->getHeight() + button_bar_height_);
    }

    void resized() override {
        auto bounds = getLocalBounds();
        auto button_bounds = bounds.removeFromBottom(button_bar_height_);
        content_->setBounds(bounds);

        done_.setBounds(button_bounds.removeFromRight(100).reduced(2, 2));
    }

    void buttonClicked(Button*) override {
        if (auto dw = findParentComponentOfClass<DialogWindow>()) {
            dw->exitModalState(1);
        }
    }

private:
    static constexpr auto button_bar_height_ = 25;

    std::unique_ptr<T> content_;

    TextButton done_{"done"};
    model::Connector<TextButton> done_connector_{&done_, this};
};

template <typename T>
auto make_done_window_ptr(std::unique_ptr<T> ptr) {
    return std::make_unique<done_window<T>>(std::move(ptr));
}

////////////////////////////////////////////////////////////////////////////////

template <typename T>
class generic_modal_callback final : public ModalComponentManager::Callback {
public:
    generic_modal_callback(T t)
            : t_{std::move(t)} {}

    void modalStateFinished(int ret) override { t_(ret); }

private:
    T t_;
};

template <typename T>
auto make_generic_modal_callback_ptr(T t) {
    return std::make_unique<generic_modal_callback<T>>(std::move(t));
}

using modal_callback = std::function<void(int)>;

template <typename T>
void begin_modal_dialog(const std::string& title,
                        std::unique_ptr<T> component,
                        modal_callback callback) {
    DialogWindow::LaunchOptions launchOptions;
    launchOptions.content.setOwned(component.release());

    launchOptions.dialogTitle = title;
    launchOptions.dialogBackgroundColour = Colours::darkgrey;
    launchOptions.escapeKeyTriggersCloseButton = true;
    launchOptions.useNativeTitleBar = true;
    launchOptions.resizable = false;
    launchOptions.useBottomRightCornerResizer = false;
    auto dw = launchOptions.create();
    dw->enterModalState(
            true,
            make_generic_modal_callback_ptr(std::move(callback)).release(),
            true);
}
