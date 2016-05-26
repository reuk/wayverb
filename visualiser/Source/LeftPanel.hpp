#pragma once

#include "BottomPanel.hpp"
#include "FullModel.hpp"
#include "SurfaceModel.hpp"

class LeftPanel : public Component, public model::BroadcastListener {
public:
    LeftPanel(model::ValueWrapper<model::FullModel>& model,
              const CuboidBoundary& aabb);

    void resized() override;

    void receive_broadcast(model::Broadcaster* cb) override;

private:
    model::ValueWrapper<model::FullModel>& model;

    model::BroadcastConnector is_rendering_connector{
        &model.render_state.is_rendering, this};

    PropertyPanel property_panel;
    BottomPanel bottom_panel;
};
