#pragma once

#include "ModelWrapper.hpp"
#include "ProgressButton.hpp"
#include "RenderState.hpp"

class BottomPanel : public Component,
                    public RenderStateManager::Listener,
                    public TextButton::Listener {
public:
    BottomPanel(RenderStateManager& render_state_manager);

    void paint(Graphics& g) override;
    void resized() override;

    void render_state_changed(RenderStateManager*, RenderState state) override;
    void render_progress_changed(RenderStateManager*, double p) override;

    void buttonClicked(Button*) override;

private:
    double progress{0};
    RenderStateManager& render_state_manager;
    model::Connector<RenderStateManager> render_state_connector{
        &render_state_manager, this};

    ProgressButton progress_button;
    model::Connector<ProgressButton> progress_connector{&progress_button, this};
};