#include "LeftPanel.hpp"

#include "SurfacePanel.hpp"
#include "Vec3Editor.hpp"

class MaterialConfigureButton : public PropertyComponent,
                                public Button::Listener {
public:
    MaterialConfigureButton(model::SurfaceWrapper& surface_wrapper)
            : PropertyComponent("material")
            , surface_wrapper(surface_wrapper)
            , button("configure...") {
        button.addListener(this);
        addAndMakeVisible(button);
    }

    virtual ~MaterialConfigureButton() noexcept {
        button.removeListener(this);
    }

    void buttonClicked(Button* b) override {
        if (b == &button) {
            auto panel = new SurfaceComponent(surface_wrapper);
            CallOutBox::launchAsynchronously(
                panel, button.getScreenBounds(), nullptr);
        }
    }

    void refresh() override {
    }

private:
    model::SurfaceWrapper& surface_wrapper;
    TextButton button;
};

LeftPanel::LeftPanel(model::Combined& combined_model,
                     RenderStateManager& render_state_manager)
        : combined_model(combined_model)
        , render_state_manager(render_state_manager)
        , bottom_panel(render_state_manager) {
    property_panel.addSection(
        "general",
        {new Vec3fProperty("source", combined_model.source),
         new Vec3fProperty("mic", combined_model.mic)});

    property_panel.addSection("materials",
                              {new MaterialConfigureButton(surface_wrapper)});

    property_panel.addSection(
        "waveguide",
        {new NumberProperty<float>("cutoff", combined_model.filter_frequency),
         new NumberProperty<float>("oversample",
                                   combined_model.oversample_ratio)});

    property_panel.addSection(
        "raytracer",
        {new NumberProperty<int>("rays", combined_model.rays),
         new NumberProperty<int>("reflections", combined_model.impulses)});

    property_panel.setOpaque(false);

    addAndMakeVisible(property_panel);
    addAndMakeVisible(bottom_panel);
}

void LeftPanel::resized() {
    auto panel_height = 30;
    property_panel.setBounds(getLocalBounds().withTrimmedBottom(panel_height));
    bottom_panel.setBounds(getLocalBounds().removeFromBottom(panel_height));
}

void LeftPanel::changeListenerCallback(ChangeBroadcaster* cb) {
}