#pragma once

#include "AudioThumbnailPane.hpp"
#include "Ruler.hpp"
#include "Transport.hpp"
#include "FileDropComponent.hpp"

class PlaybackPane : public Component,
                     public ApplicationCommandTarget,
                     public Button::Listener,
                     public PlaybackViewManager::Listener,
                     public ScrollBar::Listener {
public:
    PlaybackPane(AudioDeviceManager& audio_device_manager,
                 AudioFormatManager& audio_format_manager,
                 AudioFormatReaderSource& audio_format_reader_source,
                 const File& file);
    virtual ~PlaybackPane() noexcept;

    void resized() override;

    void getAllCommands(Array<CommandID>& commands) override;
    void getCommandInfo(CommandID command_id,
                        ApplicationCommandInfo& result) override;
    bool perform(const InvocationInfo& info) override;
    ApplicationCommandTarget* getNextCommandTarget() override;

    void buttonClicked(Button* b) override;

    void scrollBarMoved(ScrollBar* s, double new_range_start) override;

    void max_range_changed(PlaybackViewManager* r,
                           const Range<double>& range) override;
    void visible_range_changed(PlaybackViewManager* r,
                               const Range<double>& range) override;
    void current_time_changed(PlaybackViewManager* r, double time) override;

    void addListener(FileDropListener* f);
    void removeListener(FileDropListener* f);

private:
    AudioDeviceManager& audio_device_manager;
    AudioFormatManager& audio_format_manager;
    AudioFormatReaderSource& audio_format_reader_source;

    TransportViewManager playback_view_manager;
    AudioSourcePlayer audio_source_player;

    AudioThumbnailPane renderer;
    Ruler ruler;

    TextButton load_different_button;

    ToggleButton follow_playback_button;

    ScrollBar scroll_bar;

    Transport transport;

    model::Connector<TextButton> load_different_connector{
            &load_different_button, this};
    model::Connector<ToggleButton> follow_playback_connector{
            &follow_playback_button, this};
    model::Connector<ScrollBar> scroll_bar_connector{&scroll_bar, this};

    model::Connector<PlaybackViewManager> pvm_connector_0{
            &playback_view_manager, this};

    ListenerList<FileDropListener> listener_list;
};