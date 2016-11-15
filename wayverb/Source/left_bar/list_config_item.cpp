#include "list_config_item.h"
#include "AngularLookAndFeel.h"

namespace left_bar {

list_config_item::list_config_item(get_callout_component get_callout_component)
        : get_callout_component_{std::move(get_callout_component)} {
    label_.setInterceptsMouseClicks(false, false);
    
    addAndMakeVisible(label_);
    addAndMakeVisible(button_);
}

void list_config_item::set_label(const std::string& label) {
    label_.setText(label, dontSendNotification);
}

void list_config_item::resized() {
    const auto button_width = getHeight();
    auto bounds = getLocalBounds();
    button_.setBounds(bounds.removeFromRight(button_width).reduced(2, 2));
    label_.setBounds(bounds.reduced(2, 2));
}

void list_config_item::buttonClicked(Button* b) {
    CallOutBox::launchAsynchronously(get_callout_component_().release(), button_.getScreenBounds(), nullptr);
}

}  // namespace left_bar
