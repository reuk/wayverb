#include "MainComponent.hpp"

#include "combined_config_serialize.h"
#include "common/surface_serialize.h"

MainContentComponent::MainContentComponent(const File& root)
        : left_panel(render_state_manager)
        , resizer_bar(&layout_manager, 1, true)
        , right_panel(render_state_manager, root) {
    auto left_panel_width = 250;
    layout_manager.setItemLayout(
        0, left_panel_width, left_panel_width, left_panel_width);
    auto bar_width = 0;
    layout_manager.setItemLayout(1, bar_width, bar_width, bar_width);
    layout_manager.setItemLayout(2, 300, 10000, 400);

    addAndMakeVisible(left_panel);
    addAndMakeVisible(resizer_bar);
    addAndMakeVisible(right_panel);

    setSize(800, 500);

    render_state_manager.set_state(RenderState::stopped);
}

void MainContentComponent::paint(Graphics& g) {
    g.fillAll(Colours::darkgrey);
}

void MainContentComponent::resized() {
    Component* component[]{&left_panel, &resizer_bar, &right_panel};
    layout_manager.layOutComponents(
        component, 3, 0, 0, getWidth(), getHeight(), false, true);
}

void MainContentComponent::save_as_project() {
    right_panel.save_as_project();
}
