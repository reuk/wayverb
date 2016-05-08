#pragma once

#include "ProgressButton.hpp"
#include "RenderState.hpp"

class BottomPanel : public Component,
                    public RenderStateManager::Listener,
                    public TextButton::Listener {
public:
    BottomPanel(RenderStateManager& render_state_manager);
    virtual ~BottomPanel();

    void paint(Graphics& g) override;
    void resized() override;

    void render_state_changed(RenderStateManager*, RenderState state) override;
    void render_progress_changed(RenderStateManager*, double p) override;

    void buttonClicked(Button*) override;

private:
    double progress{0};
    RenderStateManager& render_state_manager;

    //    TextButton render_button;
    //    juce::ProgressBar progress_bar;
    ProgressButton progress_button;
};