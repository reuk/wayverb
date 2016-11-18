#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace left_bar {

template <typename T>
class generic_property_component final : public PropertyComponent {
public:
    template <typename... Ts>
    generic_property_component(const String& name, int height, Ts&&... ts)
            : PropertyComponent{name, height}
            , content{std::forward<Ts>(ts)...} {
        addAndMakeVisible(content);
    }

    void refresh() override {}

    T content;
};

}  // namespace left_bar
