#pragma once

#include "BottomPanel.hpp"

class LeftPanel : public Component {
public:
    LeftPanel(RenderStateManager& render_state_manager);

    void resized() override;

private:
    RenderStateManager& render_state_manager;
    PropertyPanel property_panel;
    BottomPanel bottom_panel;
};
