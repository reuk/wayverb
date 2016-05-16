#pragma once

#include "BottomPanel.hpp"
#include "ModelWrapper.hpp"
#include "SurfaceModel.hpp"

#include "PropertyComponentLAF.hpp"

class LeftPanel : public Component, public ChangeListener {
public:
    LeftPanel(model::Combined& combined_model,
              SurfaceModel& surface_model,
              RenderStateManager& render_state_manager);

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

private:
    PropertyComponentLAF pclaf;

    model::Combined& combined_model;
    model::ChangeConnector combined_connector{&combined_model, this};

    SurfaceModel& surface_model;
    model::ChangeConnector surface_connector{&surface_model, this};

    RenderStateManager& render_state_manager;

    PropertyPanel property_panel;
    BottomPanel bottom_panel;

    SurfaceModel preset_model;
};
