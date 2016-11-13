#pragma once

#include "BottomPanel.h"
#include "HelpWindow.h"
#include "async_work_queue.h"

#include "combined/model/app.h"

class LeftPanel : public Component,
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

    LeftPanel(wayverb::combined::model::app& model);

    void resized() override;

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    wayverb::combined::model::app& model_;
    async_work_queue queue_;

    PropertyPanel property_panel_;
    BottomPanel bottom_panel_;
};
