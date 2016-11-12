#pragma once

#include "BottomPanel.h"
#include "HelpWindow.h"

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

    LeftPanel();

    void resized() override;

    void addListener(Listener* l);
    void removeListener(Listener* l);

private:
    PropertyPanel property_panel;
    BottomPanel bottom_panel;
};
