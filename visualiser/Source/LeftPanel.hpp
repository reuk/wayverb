#pragma once

#include "BottomPanel.hpp"
#include "FullModel.hpp"
#include "ModelWrapper.hpp"
#include "SurfaceModel.hpp"

class LeftPanel : public Component, public ChangeListener {
public:
    LeftPanel(model::ValueWrapper<model::FullModel>& model);

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

private:
    model::ValueWrapper<model::FullModel>& model;

    model::ChangeConnector is_rendering_connector{
        &model.render_state.is_rendering, this};

    PropertyPanel property_panel;
    BottomPanel bottom_panel;
};
