#pragma once

#include "AudioThumbnailPane.hpp"
#include "FileDropComponent.hpp"
#include "Ruler.hpp"
#include "Transport.hpp"

class PlaybackPane : public Component,
                     public Button::Listener,
                     public PlaybackViewManager::Listener,
                     public ScrollBar::Listener {
public:
    class Listener {
    public:
        Listener() = default;
        Listener(const Listener&) = default;
        Listener& operator=(const Listener&) = default;
        Listener(Listener&&) noexcept = default;
        Listener& operator=(Listener&&) noexcept = default;
        virtual ~Listener() noexcept = default;

        virtual void new_file_loaded(PlaybackPane*, const File& f) = 0;
        virtual void bypass_state_changed(PlaybackPane* r, bool state) = 0;
    };

    PlaybackPane(AudioFormatManager& audio_format_manager,
                 AudioTransportSource& audio_transport_source,
                 const File& file);
    void reset_view();

    void resized() override;

    void buttonClicked(Button* b) override;

    void scrollBarMoved(ScrollBar* s, double new_range_start) override;

    void max_range_changed(PlaybackViewManager* r,
                           const Range<double>& range) override;
    void visible_range_changed(PlaybackViewManager* r,
                               const Range<double>& range) override;
    void current_time_changed(PlaybackViewManager* r, double time) override;

    void addListener(Listener* f);
    void removeListener(Listener* f);

private:
    AudioFormatManager& audio_format_manager;
    TransportViewManager transport_view_manager;
    AudioThumbnailPane renderer;
    Ruler ruler;

    TextButton load_different_button;

    ToggleButton follow_playback_button;
    ToggleButton bypass_button;

    ScrollBar scroll_bar;

    Transport transport;

    model::Connector<TextButton> load_different_connector{
            &load_different_button, this};
    model::Connector<ToggleButton> follow_playback_connector{
            &follow_playback_button, this};
    model::Connector<ToggleButton> bypass_connector{&bypass_button, this};
    model::Connector<ScrollBar> scroll_bar_connector{&scroll_bar, this};

    model::Connector<PlaybackViewManager> pvm_connector_0{
            &transport_view_manager, this};

    ListenerList<Listener> listener_list;
};