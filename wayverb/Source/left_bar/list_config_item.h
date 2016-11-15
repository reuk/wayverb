#pragma once

#include "../UtilityComponents/connector.h"

#include "../JuceLibraryCode/JuceHeader.h"

namespace left_bar {

/// Displays a name and a button for further configuration.
class list_config_item final : public Component , public TextButton::Listener {
public:
    using get_callout_component = std::function<std::unique_ptr<Component>()>;

    list_config_item(get_callout_component get_callout_component);

    void set_label(const std::string& label);

    void resized() override;

    void buttonClicked(Button* b) override;

private:
    Label label_;
    TextButton button_{"..."};
    model::Connector<TextButton> button_connector_{&button_, this};

    get_callout_component get_callout_component_;
};

}  // namespace left_bar
