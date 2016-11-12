#pragma once

#include "PlaybackViewManager.h"
#include "Playhead.h"

class Ruler : public juce::Component, public TransportViewManager::Listener {
public:
    Ruler(TransportViewManager& m);
    virtual ~Ruler() noexcept;

    void paint(juce::Graphics& g) override;

    double x_to_time(double x) const;
    double time_to_x(double time) const;

    void visible_range_changed(TransportViewManager* r,
                               const juce::Range<double>& range) override;

    void mouseEnter(const juce::MouseEvent& event) override;
    void mouseExit(const juce::MouseEvent& event) override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    void mouseDoubleClick(const juce::MouseEvent& event) override;

private:
    TransportViewManager& transport_view_manager;

    struct RulerState;
    std::unique_ptr<RulerState> ruler_state;
};
