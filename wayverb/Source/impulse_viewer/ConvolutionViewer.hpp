#pragma once

#include "DefaultAudio.hpp"
#include "FullModel.hpp"
#include "Ruler.hpp"
#include "Transport.hpp"

class AudioThumbnailPane : public Component, public ChangeListener {
public:
    AudioThumbnailPane(AudioFormatManager& audio_format_manager);

    void paint(Graphics& g) override;
    void resized() override;

    void changeListenerCallback(ChangeBroadcaster* cb) override;

    void set_reader(AudioFormatReader* new_reader, int64 hash);

    void set_visible_range(const Range<double>& range);

private:
    const AudioTransportSource& audio_transport_source;

    AudioThumbnailCache audio_thumbnail_cache;
    AudioThumbnail audio_thumbnail;

    Range<double> visible_range;
};

//----------------------------------------------------------------------------//

class ConvolutionViewer : public Component,
                          public ApplicationCommandTarget,
                          public Button::Listener,
                          public Ruler::Listener,
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

    void ruler_visible_range_changed(Ruler* r,
                                     const Range<double>& range) override;
    void scrollBarMoved(ScrollBar* s, double new_range_start) override;

private:
    AudioDeviceManager& audio_device_manager;

    AudioFormatReaderSource audio_format_reader_source;
    AudioTransportSource audio_transport_source;
    AudioSourcePlayer audio_source_player;

    AudioThumbnailPane renderer;
    Ruler ruler;

    ToggleButton follow_playback_button;

    ScrollBar scroll_bar;

    Transport transport;

    model::Connector<ToggleButton> follow_playback_connector{
            &follow_playback_button, this};
    model::Connector<Ruler> ruler_connector_0{&ruler, this};
    model::Connector<ScrollBar> scroll_bar_connector{&scroll_bar, this};
};
