#pragma once

#include "BottomPanel.hpp"
#include "HelpWindow.hpp"
#include "SurfaceModel.hpp"

/*
class LeftPanel : public Component,
                  public model::BroadcastListener,
                  public SettableHelpPanelClient {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void left_panel_debug_show_closest_surfaces(
                const LeftPanel*) = 0;
        virtual void left_panel_debug_show_boundary_types(const LeftPanel*) = 0;
        virtual void left_panel_debug_hide_debug_mesh(const LeftPanel*) = 0;
    };

    LeftPanel(model::ValueWrapper<model::FullModel>& model,
              const geo::box& aabb);

    void resized() override;

    void receive_broadcast(model::Broadcaster* b) override;

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    model::ValueWrapper<model::FullModel>& model;

    model::ValueWrapper<int> waveguide_sampling_rate_wrapper{nullptr, 0};

    model::BroadcastConnector filter_frequency_connector{
            &model.persistent.app.filter_frequency, this};
    model::BroadcastConnector oversample_ratio_connector{
            &model.persistent.app.oversample_ratio, this};

    model::BroadcastConnector is_rendering_connector{
            &model.render_state.is_rendering, this};

    PropertyPanel property_panel;
    BottomPanel bottom_panel;

    ListenerList<Listener> listener_list;
};
*/
