#pragma once

#include "FullModel.hpp"
#include "PlaybackViewManager.hpp"
#include "Playhead.hpp"

class Ruler : public Component, public PlaybackViewManager::Listener {
public:
    Ruler(PlaybackViewManager& m);
    virtual ~Ruler() noexcept;

    void paint(Graphics& g) override;
    void resized() override;

    double x_to_time(double x) const;
    double time_to_x(double time) const;

    void max_range_changed(PlaybackViewManager* r,
                           const Range<double>& range) override;
    void visible_range_changed(PlaybackViewManager* r,
                               const Range<double>& range) override;
    void current_time_changed(PlaybackViewManager* r, double time) override;

    void mouseEnter(const MouseEvent& event) override;
    void mouseExit(const MouseEvent& event) override;
    void mouseDown(const MouseEvent& event) override;
    void mouseUp(const MouseEvent& event) override;
    void mouseDrag(const MouseEvent& event) override;
    void mouseDoubleClick(const MouseEvent& event) override;

private:
    PlaybackViewManager& playback_view_manager;
    model::Connector<PlaybackViewManager> view_connector{&playback_view_manager,
                                                         this};

    struct RulerState;
    std::unique_ptr<RulerState> ruler_state;
};