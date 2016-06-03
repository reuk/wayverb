#pragma once

#include "BottomPanel.hpp"
#include "FullModel.hpp"
#include "HelpWindow.hpp"
#include "SurfaceModel.hpp"

class LeftPanel : public Component,
                  public model::BroadcastListener,
                  public SettableHelpPanelClient {
public:
    LeftPanel(model::ValueWrapper<model::FullModel>& model,
              const CuboidBoundary& aabb);

    void resized() override;

    void receive_broadcast(model::Broadcaster* b) override;

private:
    model::ValueWrapper<model::FullModel>& model;

    int waveguide_sampling_rate{0};
    model::ValueWrapper<int> waveguide_sampling_rate_wrapper{
        nullptr, waveguide_sampling_rate};

    model::BroadcastConnector filter_frequency_connector{
        &model.persistent.combined.filter_frequency, this};
    model::BroadcastConnector oversample_ratio_connector{
        &model.persistent.combined.oversample_ratio, this};

    model::BroadcastConnector is_rendering_connector{
        &model.render_state.is_rendering, this};

    PropertyPanel property_panel;
    BottomPanel bottom_panel;
};
