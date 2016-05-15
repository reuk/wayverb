#include "MainComponent.hpp"

#include "combined_config_serialize.h"
#include "common/surface_serialize.h"

#include "LoadFiles.hpp"

MainContentComponent::MainContentComponent(const File& root)
        : combined_model(nullptr, load_config(root))
        , scene_data(load_model(root, load_materials(root)))
        , surface_model(nullptr, scene_data.get_materials())
        , left_panel(combined_model, surface_model, render_state_manager)
        , resizer_bar(&layout_manager, 1, true)
        , right_panel(combined_model, scene_data, render_state_manager) {
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
