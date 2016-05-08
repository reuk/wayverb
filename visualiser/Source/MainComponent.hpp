#pragma once

#include "LeftPanel.hpp"
#include "RightPanel.hpp"

class MainContentComponent final : public Component {
public:
    MainContentComponent(const File& root);

    void paint(Graphics& g) override;
    void resized() override;

    void save_as_project();

private:
    RenderStateManager render_state_manager;

    StretchableLayoutManager layout_manager;

    LeftPanel left_panel;
    StretchableLayoutResizerBar resizer_bar;
    RightPanel right_panel;
};
