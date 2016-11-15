#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace left_bar {

/// Displays a name and a button for further configuration.
class list_config_item final : public Component {
public:
    using get_callout_component = std::function<std::unique_ptr<Component>()>;

    list_config_item(get_callout_component get_callout_component);

    void set_label(const std::string& label);

    void resized() override;

private:
    Label label_;
    TextButton button_{"..."};
    get_callout_component get_callout_component_;
};

}  // namespace left_bar
