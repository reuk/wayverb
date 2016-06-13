#pragma once

#include "DefaultAudio.hpp"
#include "FullModel.hpp"
#include "Playhead.hpp"
#include "Ruler.hpp"
#include "Transport.hpp"

class AudioThumbnailPane : public Component,
                           public ChangeListener,
                           public PlaybackViewManager::Listener {
public:
    AudioThumbnailPane(AudioFormatManager& audio_format_manager,
                       PlaybackViewManager& playback_view_manager);

    void paint(Graphics& g) override;
    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

    void set_reader(AudioFormatReader* new_reader, int64 hash);

    void max_range_changed(PlaybackViewManager* r,
                           const Range<double>& range) override;
    void visible_range_changed(PlaybackViewManager* r,
                               const Range<double>& range) override;
    void current_time_changed(PlaybackViewManager* r, double time) override;

private:
    double time_to_x(double t) const;
    void position_playhead();

    const AudioTransportSource& audio_transport_source;
    PlaybackViewManager& playback_view_manager;
    model::Connector<PlaybackViewManager> view_connector{&playback_view_manager,
                                                         this};

    AudioThumbnailCache audio_thumbnail_cache;
    AudioThumbnail audio_thumbnail;
    Playhead playhead;
};

//----------------------------------------------------------------------------//

class ConvolutionViewer : public Component,
                          public ApplicationCommandTarget,
                          public Button::Listener,
                          public PlaybackViewManager::Listener,
                          public ScrollBar::Listener,
                          public Timer {
public:
    ConvolutionViewer(AudioDeviceManager& audio_device_manager,
                      AudioFormatManager& audio_format_manager,
                      const File& file);
    virtual ~ConvolutionViewer() noexcept;

    void resized() override;

    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

    void buttonClicked(Button* b) override;

    void timerCallback() override;

    void scrollBarMoved(ScrollBar* s, double new_range_start) override;

    void max_range_changed(PlaybackViewManager* r,
                           const Range<double>& range) override;
    void visible_range_changed(PlaybackViewManager* r,
                               const Range<double>& range) override;
    void current_time_changed(PlaybackViewManager* r, double time) override;

private:
    AudioDeviceManager& audio_device_manager;

    AudioFormatReaderSource audio_format_reader_source;
    AudioTransportSource audio_transport_source;
    AudioSourcePlayer audio_source_player;

    PlaybackViewManager playback_view_manager;

    AudioThumbnailPane renderer;
    Ruler ruler;

    ToggleButton follow_playback_button;

    ScrollBar scroll_bar;

    Transport transport;

    model::Connector<ToggleButton> follow_playback_connector{
            &follow_playback_button, this};
    model::Connector<ScrollBar> scroll_bar_connector{&scroll_bar, this};

    model::Connector<PlaybackViewManager> pvm_connector_0{
            &playback_view_manager, this};
};
