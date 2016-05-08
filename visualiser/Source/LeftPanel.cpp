#include "LeftPanel.hpp"

#include "Vec3Editor.hpp"

LeftPanel::LeftPanel(RenderStateManager& render_state_manager)
        : render_state_manager(render_state_manager)
        , bottom_panel(render_state_manager) {
    property_panel.addSection(
        "general",
        {
            new Vec3Property("source"), new Vec3Property("mic"),
        });
    property_panel.addSection("materials",
                              {

                              });
    property_panel.addSection(
        "waveguide",
        {
            new NumberProperty("cutoff"), new NumberProperty("oversample"),
        });
    property_panel.addSection(
        "raytracer",
        {
            new NumberProperty("rays"), new NumberProperty("reflections"),
        });

    addAndMakeVisible(property_panel);
    addAndMakeVisible(bottom_panel);
}

void LeftPanel::resized() {
    auto panel_height = 30;
    property_panel.setBounds(
        getLocalBounds().withTrimmedBottom(panel_height));
    bottom_panel.setBounds(getLocalBounds().removeFromBottom(panel_height));
}