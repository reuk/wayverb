#pragma once

#include "BottomPanel.hpp"
#include "ModelWrapper.hpp"

class LeftPanel : public Component, public ChangeListener {
public:
    LeftPanel(model::Combined& combined_model,
              RenderStateManager& render_state_manager);

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

private:
    model::Combined& combined_model;
    model::ChangeConnector model_connector{&combined_model, this};

    RenderStateManager& render_state_manager;

    Surface test_surface;
    model::SurfaceWrapper surface_wrapper{nullptr, test_surface};
    model::ChangeConnector surface_connector{&surface_wrapper, this};

    PropertyPanel property_panel;
    BottomPanel bottom_panel;
};
