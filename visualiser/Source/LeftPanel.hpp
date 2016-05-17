#pragma once

#include "BottomPanel.hpp"
#include "ModelWrapper.hpp"
#include "SurfaceModel.hpp"

#include "PropertyComponentLAF.hpp"

class LeftPanel : public Component, public ChangeListener {
public:
    LeftPanel(model::Combined& combined_model,
              model::Surfaces& surface_model,
              RenderStateManager& render_state_manager);

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

private:
    PropertyComponentLAF pclaf;

    model::Combined& combined_model;
    model::ChangeConnector combined_connector{&combined_model, this};

    model::Surfaces& surface_model;
    model::ChangeConnector surface_connector{&surface_model, this};

    model::ValueWithWrapper<model::FullReceiverConfig> receiver_config{nullptr};

    RenderStateManager& render_state_manager;

    PropertyPanel property_panel;
    BottomPanel bottom_panel;

    model::Surfaces preset_model;
};
